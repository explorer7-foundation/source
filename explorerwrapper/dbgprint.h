#pragma once
#include "common.h"
#include <cstdint>

extern "C" void * _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

extern HINSTANCE g_hInstance;

void dbgvprintf(LPCWSTR format, void* _argp);
void dbgprintf(LPCWSTR format, ...);

BOOL WINAPI ChangeImportedAddress_FARPROC( HMODULE hModule, LPSTR modulename, FARPROC origfunc, FARPROC newfunc );
__declspec(noinline) BOOL WINAPI ChangeImportedAddress_ORDINAL( HMODULE hModule, LPSTR modulename, ULONGLONG origOrdinal, FARPROC newfunc );
__declspec(noinline) BOOL WINAPI ChangeExportedAddress_ORDINAL( HMODULE hModule, ULONGLONG origOrdinal, LPCSTR newForward);
#define ChangeImportedAddress(hModule,modulename,origproc,newproc) ChangeImportedAddress_FARPROC(hModule,modulename,(FARPROC)origproc,(FARPROC)newproc)
#define ChangeImportedAddressORDINAL(hModule,modulename,origproc,newproc) ChangeImportedAddress_ORDINAL(hModule,modulename,origproc,(FARPROC)newproc)
#define ChangeExportedAddress_ORDINAL(hModule,origproc,newproc) ChangeExportedAddress_ORDINAL(hModule,origproc,newproc)

static int
IsSpaceCS(char ch)
{
	return ((ch == ' ') || (ch == '\t'));
}

static int
IsDigitCS(char ch)
{
	return (((ch >= '0') && (ch <= '9')));
}

static int
isAlphaCS(char c)
{
	/*
	 * Depends on ASCII-like character ordering.
	 */
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ? 1 : 0);
}

static int
isUpperCS(int c)
{
	return (c >= 'A' && c <= 'Z');
}

static unsigned long
strtoulCUSTOM(const char* nptr, char** endptr, int base)
{
	const char* s = nptr;
	unsigned long acc;
	int c;
	unsigned long cutoff;
	int neg = 0, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (IsSpaceCS(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
		c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (IsDigitCS(c))
			c -= '0';
		else if (isAlphaCS(c))
			c -= isUpperCS(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
		//errno = ERANGE;
	}
	else if (neg)
		acc = -(long)acc;
	if (endptr != 0)
		*endptr = (char*)(any ? s - 1 : nptr);
	return (acc);
}

template <class T>
struct wiktorArray
{
	int size;
	T* data;

	wiktorArray()
	{
		size = 0;
		data = NULL;
	}

	~wiktorArray()
	{
		if (data)
		{
			free(data);
			data = NULL;
		}
	}

	void push_back(T InputData)
	{
		void* newData = realloc(data, sizeof(T) * (size + 1));
		if (newData)
		{
			data = (T*)newData;
			data[size++] = InputData;
		}
	}
};

static wiktorArray<int>* patternToByte(const char* pattern)
{
	auto bytes = new wiktorArray<int>();
	const auto start = const_cast<char*>(pattern);
	const auto end = const_cast<char*>(pattern) + strlen(pattern);

	for (auto current = start; current < end; ++current)
	{
		if (*current == '?')
		{
			++current;
			if (*current == '?')
				++current;
			bytes->push_back(-1);
		}
		else { bytes->push_back(strtoulCUSTOM(current, &current, 16)); }
	}
	return bytes;
}

static uintptr_t FindPattern(uintptr_t baseAddress, const char* signature)
{
	const auto dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	const auto ntHeaders = (PIMAGE_NT_HEADERS)((unsigned char*)baseAddress + dosHeader->e_lfanew);

	const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto patternBytes = patternToByte(signature);
	const auto scanBytes = reinterpret_cast<unsigned char*>(baseAddress);

	const auto s = patternBytes->size;
	const auto d = patternBytes->data;

	for (size_t i = 0; i < sizeOfImage - s; ++i)
	{
		bool found = true;
		for (size_t j = 0; j < s; ++j)
		{
			if (scanBytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);

			delete patternBytes;
			return address;
		}
	}

	delete patternBytes;

	return NULL;
}

//allocates memory close enough to the provided targetAddr argument to be reachable
	//from the targetAddr by a 32 bit jump instruction
static void* _AllocatePageNearAddress(void* targetAddr)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

	uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); //round down to nearest page boundary
	uint64_t minAddr = min(startAddr - 0x7FFFFF00, (uint64_t)sysInfo.lpMinimumApplicationAddress);
	uint64_t maxAddr = max(startAddr + 0x7FFFFF00, (uint64_t)sysInfo.lpMaximumApplicationAddress);

	uint64_t startPage = (startAddr - (startAddr % PAGE_SIZE));

	uint64_t pageOffset = 1;
	while (1)
	{
		uint64_t byteOffset = pageOffset * PAGE_SIZE;
		uint64_t highAddr = startPage + byteOffset;
		uint64_t lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

		bool needsExit = highAddr > maxAddr && lowAddr < minAddr;

		if (highAddr < maxAddr)
		{
			void* outAddr = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr)
				return outAddr;
		}

		if (lowAddr > minAddr)
		{
			void* outAddr = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr != nullptr)
				return outAddr;
		}

		pageOffset++;

		if (needsExit)
		{
			break;
		}
	}

	return nullptr;
}

static uint32_t _WriteAbsoluteJump64(void* absJumpMemory, void* addrToJumpTo)
{
	//this writes the absolute jump instructions into the memory allocated near the target
	//the E8 jump installed in the target function will jump to here

	//r10 is chosen here because it's a volatile register according to the windows x64 calling convention, 
	//but is not used for return values (like rax) or function arguments (like rcx, rdx, r8, r9)
	uint8_t absJumpInstructions[] = { 0x49, 0xBA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //mov 64 bit value into r10
										0x41, 0xFF, 0xE2 }; //jmp r10

	uint64_t addrToJumpTo64 = (uint64_t)addrToJumpTo;
	memcpy(&absJumpInstructions[2], &addrToJumpTo64, sizeof(addrToJumpTo64));
	memcpy(absJumpMemory, absJumpInstructions, sizeof(absJumpInstructions));
	return sizeof(absJumpInstructions);
}

inline void DetourCall(void* Target, void* Detour)
{
	void* relayFuncMemory = _AllocatePageNearAddress(Target);

	_WriteAbsoluteJump64(relayFuncMemory, Detour); //write relay func instructions

	DWORD oldProtect;
	BOOL success = VirtualProtect(Target, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

	//32 bit relative call opcode is E8, takes 1 32 bit operand for jump offset
	uint8_t jmpInstruction[5] = { 0xE8, 0x0, 0x0, 0x0, 0x0 };

	//to fill out the last 4 bytes of callInstruction, we need the offset between 
	//the relay function and the instruction immediately AFTER the jmp instruction
	const uint64_t relAddr = (uint64_t)relayFuncMemory - ((uint64_t)Target + sizeof(jmpInstruction));
	memcpy(jmpInstruction + 1, &relAddr, 4);

	//install the hook
	memcpy(Target, jmpInstruction, sizeof(jmpInstruction));
}

//Ittr: Consolidated function for pattern byte replacements.
static void ChangeImportedPattern(void* dllPattern, const unsigned char* newBytes, SIZE_T size) //thank you wiktor
{
	if (dllPattern)
	{
		DWORD old;
		VirtualProtect(dllPattern, size, PAGE_EXECUTE_READWRITE, &old);
		memcpy(dllPattern, newBytes, size);
		VirtualProtect(dllPattern, size, old, 0);
	}
}
