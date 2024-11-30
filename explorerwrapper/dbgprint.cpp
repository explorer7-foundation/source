#include "dbgprint.h"

void dbgvprintf(LPCWSTR format, void* _argp)
{
	WCHAR msg[1024];
	va_list argp = (va_list) _argp;
	int cnt = wvsprintfW(msg, format, argp);

	OutputDebugStringW(msg);
}

void dbgvprintfA(LPCSTR format, void* _argp)
{
    CHAR msg[1024];
    va_list argp = (va_list)_argp;
    int cnt = wvsprintfA(msg, format, argp);

    OutputDebugStringA(msg);
}

void dbgprintf(LPCWSTR format, ...)
{
    //format = wcscat(const_cast<wchar_t*>(format),L"\n");
	va_list argp;
	va_start(argp, format);
	dbgvprintf(format, argp);
	va_end(argp);
}

void dbgprintfA(LPCSTR format, ...)
{
    //format = wcscat(const_cast<wchar_t*>(format),L"\n");
    va_list argp;
    va_start(argp, format);
    dbgvprintfA(format, argp);
    va_end(argp);
}

char* concat(const char* s1, const char* s2)
{
    char* result = (char*)malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

BOOL WINAPI ChangeImportedAddress_FARPROC( HMODULE hModule, LPSTR modulename, FARPROC origfunc, FARPROC newfunc )
{
	DWORD_PTR lpFileBase = (DWORD_PTR)hModule;	
	if ( !lpFileBase || !modulename || !origfunc || !newfunc ) return FALSE;	
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_IMPORT_DESCRIPTOR pImportDir;
    DWORD_PTR* pFunctions;    
    LPSTR name;
    DWORD oldpr;
    
    
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    pNTHeader = (PIMAGE_NT_HEADERS)( lpFileBase + dosHeader->e_lfanew );    
    pImportDir = (PIMAGE_IMPORT_DESCRIPTOR) ( lpFileBase + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress );
    
    while ( pImportDir->Name )
    {		
		name = (LPSTR) (lpFileBase + pImportDir->Name);

		if ( lstrcmpiA( name, modulename ) == 0 ) break;

        //auto toprint = concat(name, "\n");
        //OutputDebugStringA(toprint);
        //free(toprint);
        //dbgprintf(L"Redirected in %s\n", modulename);

		pImportDir++;
    }
    if ( !pImportDir->Name ) return FALSE;
    
    pFunctions = (DWORD_PTR*)(lpFileBase + pImportDir->FirstThunk);
    
    while ( *pFunctions )
    {
		if ( *pFunctions == (DWORD_PTR)origfunc ) break;
		pFunctions++;
    }
    if ( !*pFunctions ) return FALSE;
    
	VirtualProtect( pFunctions, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &oldpr );
	*pFunctions = (DWORD_PTR)newfunc;
	VirtualProtect( pFunctions, sizeof(DWORD_PTR), oldpr, &oldpr );
    return TRUE;
}

BOOL WINAPI ChangeImportedAddress_ORDINAL(HMODULE hModule, LPSTR modulename, ULONGLONG origOrdinal, FARPROC newfunc)
{
    DWORD_PTR lpFileBase = (DWORD_PTR)hModule;
    if (!lpFileBase || !newfunc || !modulename) return FALSE;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_IMPORT_DESCRIPTOR pImportDir;
    DWORD_PTR* pFunctions;
    LPSTR name;
    DWORD oldpr;


    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    pNTHeader = (PIMAGE_NT_HEADERS)(lpFileBase + dosHeader->e_lfanew);
    pImportDir = (PIMAGE_IMPORT_DESCRIPTOR)(lpFileBase + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (pImportDir->Name)
    {
        name = (LPSTR)(lpFileBase + pImportDir->Name);
        if (!name) continue;
        //auto toprint = concat(name, "\n");
        //if (toprint)
        //{
        //    //OutputDebugStringA(toprint);
        //    //free(toprint);
        //}
    
        if (lstrcmpiA(name, modulename) == 0) break;
    
        
        
        //dbgprintf(L"Redirected in %s\n", modulename);
    
        pImportDir++;
    }
    if (!pImportDir->Name) return FALSE;

    pFunctions = (DWORD_PTR*)(lpFileBase + pImportDir->FirstThunk);

    PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(lpFileBase + pImportDir->FirstThunk);
    PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA)(lpFileBase + pImportDir->OriginalFirstThunk);

    // Iterate through the thunks
    while (originalThunk->u1.AddressOfData) {
        if (originalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
            if (originalThunk->u1.Ordinal == origOrdinal)
            {
                auto pFunc = reinterpret_cast<DWORD_PTR*>(originalThunk);
                VirtualProtect(pFunc, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &oldpr);
                *pFunc = (DWORD_PTR)newfunc;
                VirtualProtect(pFunc, sizeof(DWORD_PTR), oldpr, &oldpr);
               // OutputDebugStringA("Good\n");
            }
            //dbgprintf(L"ordinal %i\n", originalThunk->u1.Ordinal);
            // Import by Ordinal
            //std::cout << "  Ordinal: " << (originalThunk->u1.Ordinal & 0xFFFF) << std::endl;
        }
        //else {
        //    // Import by Name
        //    PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)(lpFileBase + originalThunk->u1.AddressOfData);
        //    std::cout << "  Function: " << importByName->Name << std::endl;
        //}
        thunk++;
        originalThunk++;
    }

    //VirtualProtect(pFunctions, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &oldpr);
    //*pFunctions = (DWORD_PTR)newfunc;
    //VirtualProtect(pFunctions, sizeof(DWORD_PTR), oldpr, &oldpr);
    return FALSE;
}

BOOL WINAPI ChangeExportedAddress_ORDINAL(HMODULE hModule, ULONGLONG origOrdinal, LPCSTR newForward)
{
    DWORD_PTR lpFileBase = (DWORD_PTR)hModule;
    if (!lpFileBase || !newForward) return FALSE;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_EXPORT_DIRECTORY pImportDir;
    //LPSTR name;
    DWORD oldpr;

    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    pNTHeader = (PIMAGE_NT_HEADERS)(lpFileBase + dosHeader->e_lfanew);
    pImportDir = (PIMAGE_EXPORT_DIRECTORY)(lpFileBase + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    DWORD* nameRvas = (DWORD*)(lpFileBase + pImportDir->AddressOfNames);
    WORD* ordinals = (WORD*)(lpFileBase + pImportDir->AddressOfNameOrdinals);
    DWORD* functionRvas = (DWORD*)(lpFileBase + pImportDir->AddressOfFunctions);

    DWORD VirtualAddress = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD Size = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;


    for (DWORD i = 0; i < pImportDir->NumberOfFunctions; i++) 
    {
        DWORD ordinal = pImportDir->Base + i;

        char name[256] = "NO NAME";

        size_t x = 0;
        for (x = 0; x < pImportDir->NumberOfNames; ++x)
        {
            if (ordinals[x] == i)
            {
                char* functionName = (char*)(lpFileBase + nameRvas[x]);
                strcpy(name, functionName);
                break;
            }
        }
        DWORD functionRva = functionRvas[i];

        FARPROC functionAddress = (FARPROC)(lpFileBase + functionRva);
        if (functionRva >= VirtualAddress && functionRva < VirtualAddress + Size) {
            char* forwardedTo = (char*)functionAddress;
            //dbgprintfA("Forwarded Export: ordinal %i %s %s\n", ordinal, name, forwardedTo);
            if (origOrdinal == ordinal)
            {
                size_t len = strlen(newForward) + 1;
                VirtualProtect(forwardedTo, len * sizeof(CHAR), PAGE_EXECUTE_READWRITE, &oldpr);
                memcpy((void*)(lpFileBase + functionRva), newForward, sizeof(CHAR) * len);
                VirtualProtect(forwardedTo, len * sizeof(CHAR), oldpr, 0);
                //dbgprintfA("forwardedTo %s\n", forwardedTo);
                break;

            }
        }
        else
        {
            
            //dbgprintfA("Exported Function: %s,%i \n", name, ordinal);
        }

    }


    return FALSE;
}
