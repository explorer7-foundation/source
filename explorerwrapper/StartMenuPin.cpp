#include "StartMenuPin.h"
#include "dbgprint.h"
#include "OSVersion.h"

CreateInstance_API CreateStartMenuPinInstance;
PSTARTPINVTBL origStartPinVtbl;
bool bFinished = false;
//PSTARTPINVTBL newStartPinVtbl;

HMODULE h_shell32;

const LPWSTR sz_StartPage2 = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage2";
const LPWSTR sz_StartPin = L"startpin";
const LPWSTR sz_StartUnpin = L"startunpin";

int WINAPI Shell32_LoadString(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
	int result;
	if (hInstance == h_shell32 && (uID == 0x1505 || uID == 0x1506 || uID == 0x1508 || uID == 0x1509))
	{
		//try loading shell32.dll.mui
		WCHAR locales[100];
		ULONG clangs;
		ULONG cblocales = 100;
		GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &clangs, locales, &cblocales);
		WCHAR muipath[MAX_PATH];
		GetModuleFileName(NULL, muipath, MAX_PATH);
		PathRemoveFileSpec(muipath);
		PathAddBackslash(muipath);
		PathAppend(muipath, locales);
		PathAddBackslash(muipath);
		PathAppend(muipath, L"shell32.dll.mui");
		hInstance = LoadLibraryEx(muipath, 0, LOAD_LIBRARY_AS_DATAFILE);
		int result = LoadStringW(hInstance, uID, lpBuffer, nBufferMax);
		FreeLibrary(hInstance);
		if (result == 0) //fallback - load from us
			result = LoadStringW(g_hInstance, uID, lpBuffer, nBufferMax);
	}
	else
		result = LoadStringW(hInstance, uID, lpBuffer, nBufferMax);
	return result;
}

static LRESULT RegGetDWORD(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	DWORD sz = 4;
	return SHRegGetValueW(key, subkey, value, SRRF_RT_REG_DWORD, NULL, dwVal, &sz);
}

static LRESULT RegSetDWORD(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	return SHSetValueW(key, subkey, value, REG_DWORD, dwVal, 4);
}

void CStartMenuPin::QueryInterface(){};
void CStartMenuPin::AddRef(){};
void CStartMenuPin::Release(){};
void CStartMenuPin::Initialize(){};
void CStartMenuPin::NotifyPinListChange(){};
void CStartMenuPin::Unimpl1(){};
void CStartMenuPin::UpgradeItem(){};
void CStartMenuPin::IsAcceptableTarget(){};
void CStartMenuPin::Unimpl2(){};
void CStartMenuPin::SendPinRearrangeSQM(){};
void CStartMenuPin::GetPinnedAppSQMEventID(){};
void CStartMenuPin::AppliesTo(){};
void CStartMenuPin::v_GetPinListMutexName() {};

LRESULT CStartMenuPin::SetChangeCount(ULONG value)
{
	//dbgprintf(L"SetChangeCount %i",value);
	return RegSetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesChanges", &value);
}

IStream* CStartMenuPin::OpenPinRegStream(ULONG grfMode)
{
	//dbgprintf(L"OpenPinRegStream %i", grfMode);
	return SHOpenRegStream2W(HKEY_CURRENT_USER, sz_StartPage2, L"Favorites", grfMode);
}

IStream* CStartMenuPin::OpenLinksRegStream(ULONG grfMode)
{
	//dbgprintf(L"OpenLinksRegStream %i", grfMode);
	return SHOpenRegStream2W(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesResolve", grfMode);
}

LRESULT CStartMenuPin::GetPinStreamVersion()
{
	DWORD value = 0;
	RegGetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesVersion", &value);
	//dbgprintf(L"GetPinStreamVersion %i", value);
	return value;
}

LRESULT CStartMenuPin::SetPinStreamVersion(ULONG value)
{
	//dbgprintf(L"SetPinStreamVersion %i", value);
	return RegSetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesVersion", &value);
}

LRESULT CStartMenuPin::GetBackupSubDirName(LPWSTR szOut, UINT cbLen)
{
	//dbgprintf(L"GetBackupSubDirName");
	lstrcpyn(szOut, L"StartMenu", cbLen);
	return S_OK; //...right?
}

DWORD CStartMenuPin::IsRestricted()
{
	//dbgprintf(L"IsRestricted");
	return SHRestricted(REST_NOSMPINNEDLIST);
}

LRESULT(__fastcall* fGetMenuStringID)(void*, UINT*);
LRESULT CStartMenuPin::GetMenuStringID(UINT* w)
{
	//dbgprintf(L"w %i", *w);
	fGetMenuStringID(this, w);
	(*w) -= 5;
	return S_OK;
}

int CStartMenuPin::GetHelpText(unsigned __int64 id, LPWSTR buf, UINT nCharMax)
{
	return Shell32_LoadString(h_shell32, id + 0x1508ul, buf, nCharMax);
}

WCHAR* CStartMenuPin::GetVerb(UINT op)
{
	//dbgprintf(L"getverb %i", op);
	if (op == 0) return sz_StartPin;
	if (op == 1) return sz_StartUnpin;
	return NULL;
}

LRESULT CStartMenuPin::GetChangeCount(ULONG* pdwVal)
{
	//dbgprintf(L"GetChangeCount ");
	*pdwVal = 0;
	return RegGetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesChanges", pdwVal);
}

__int64 CStartMenuPin::GetRemovedChangeCount()
{
	//dbgprintf(L"GetRemovedChangeCount ");
	DWORD value = 0;
	RegGetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesRemovedChanges", &value);
	return value;
}

LRESULT CStartMenuPin::SetRemovedChangeCount(ULONG value)
{
	//dbgprintf(L"SetRemovedChangeCount %i",value);
	return RegSetDWORD(HKEY_CURRENT_USER, sz_StartPage2, L"FavoritesRemovedChanges", &value);
}

static void* DetourVtable(void* vtable, int offset, void* FunctionPtr)
{
	uintptr_t* og = (uintptr_t*)((uintptr_t)vtable + offset);

	DWORD dwProtection;
	VirtualProtect(og, 8, PAGE_EXECUTE_READWRITE, &dwProtection);

	uintptr_t originalfunction = *og;
	*og = (uintptr_t)FunctionPtr;

	VirtualProtect(og, 8, dwProtection, NULL);

	return (void*)originalfunction;
}

template <class T>
static inline void* GetMemberFuncPtr(T Func) { return reinterpret_cast<void*&>(Func); }
#define MEMBER_FUNC(a) GetMemberFuncPtr(&a)

#pragma function(memcpy)
HRESULT WINAPI NewCreateStartMenuPinInstance(PVOID dummy,REFIID riid,PVOID* ppv)
{
	dbgprintf(L"StartMenuPin: NewCreateStartMenuPinInstance");
	IUnknown* pinobj;
	HRESULT rslt = CreateStartMenuPinInstance(dummy,IID_IShellExtInit,(PVOID*)&pinobj);
	if ( SUCCEEDED(rslt))
	{
		int SetChangeCountIndex = 4 * sizeof(uintptr_t);
		int OpenPinRegStreamIndex = 5 * sizeof(uintptr_t);
		int OpenLinksRegStreamIndex = 6 * sizeof(uintptr_t);
		int GetPinStreamVersionIndex = 8 * sizeof(uintptr_t);
		int SetPinStreamVersionIndex = 9 * sizeof(uintptr_t);
		int GetBackupSubDirNameIndex = 12 * sizeof(uintptr_t);
		int IsRestrictedIndex = 14 * sizeof(uintptr_t);
		int GetMenuStringIDIndex = 16 * sizeof(uintptr_t);
		int GetHelpTextIndex = 17 * sizeof(uintptr_t);
		int GetChangeCountIndex = 18 * sizeof(uintptr_t);
		int GetVerbIndex = 19 * sizeof(uintptr_t);
		int SetRemovedChangeCountIndex = 21 * sizeof(uintptr_t);
		int GetRemovedChangeCountIndex = 22 * sizeof(uintptr_t);

		PSTARTPINOBJ startobj = (PSTARTPINOBJ)pinobj;
		PSTARTPINVTBL ogTable = startobj->pStartPinVtbl;
		dbgprintf(L"CreateStartMenuPin pStartPinVtbl %p %p setchangecount %p",startobj,startobj->pStartPinVtbl, startobj->pStartPinVtbl->SetChangeCount);
		static CStartMenuPin* HackHack = new CStartMenuPin();
		startobj->pStartPinVtbl = *(PSTARTPINVTBL*)(HackHack);

		fGetMenuStringID = decltype(fGetMenuStringID)(ogTable->GetMenuStringID);
		if (!bFinished) //only needs to be done once
		{
			DetourVtable(startobj->pStartPinVtbl, 0 * 8, ogTable->QueryInterface);
			DetourVtable(startobj->pStartPinVtbl, 1 * 8, ogTable->AddRef);
			DetourVtable(startobj->pStartPinVtbl, 2 * 8, ogTable->Release);
			DetourVtable(startobj->pStartPinVtbl, 3 * 8, ogTable->Initialize);
			DetourVtable(startobj->pStartPinVtbl, 7 * 8, ogTable->NotifyPinListChange);
			DetourVtable(startobj->pStartPinVtbl, 10 * 8, ogTable->Unimpl1);
			DetourVtable(startobj->pStartPinVtbl, 11 * 8, ogTable->UpgradeItem);
			DetourVtable(startobj->pStartPinVtbl, 13 * 8, ogTable->IsAcceptableTarget);
			DetourVtable(startobj->pStartPinVtbl, 15 * 8, ogTable->Unimpl2);
			DetourVtable(startobj->pStartPinVtbl, 20 * 8, ogTable->SendPinRearrangeSQM);
			DetourVtable(startobj->pStartPinVtbl, 23 * 8, ogTable->GetPinnedAppSQMEventID);
			if (g_osVersion.BuildNumber() >= 17763)
			{
				DetourVtable(startobj->pStartPinVtbl, 24 * 8, ogTable->AppliesTo);
				DetourVtable(startobj->pStartPinVtbl, 25 * 8, ogTable->v_GetPinListMutexName);
			}
			bFinished = true;
		}
	}
	rslt = pinobj->QueryInterface(riid, ppv);
	pinobj->Release();
	return rslt;
}

bool IsProcessAnExplorerHook()
{
	return true;
}

void StartMenuPin_PatchShell32() 
{	
	h_shell32 = GetModuleHandle(L"shell32.dll");
	ChangeImportedAddress(h_shell32,"api-ms-win-core-libraryloader-l1-2-0.dll",GetProcAddress(GetModuleHandle(L"kernelbase.dll"),"LoadStringW"),Shell32_LoadString);
	ChangeImportedAddress(GetModuleHandle(0), "shell32.dll", GetProcAddress(GetModuleHandle(L"shell32.dll"), "IsProcessAnExplorer"), IsProcessAnExplorerHook);

	DWORD_PTR addr = FindPattern((uintptr_t)h_shell32, "48 85 C0 0F 85 ?? ?? ?? ?? 45 8B C5 4C 8D 15 ?? ?? ?? ??");
	if (addr)
		addr += 15;
	else
	{
		addr = FindPattern((uintptr_t)h_shell32, "41 8B FD 48 8D 1D ?? ?? ?? ?? 4C 8D 3D");
		if (addr)
			addr += 12;
		else
		{
			dbgprintf(L"StartMenuPin_PatchShell32 SIG DID NOT WORK!!!\n");
			dbgprintf(L"StartMenuPin_PatchShell32 SIG DID NOT WORK!!!\n");
			dbgprintf(L"StartMenuPin_PatchShell32 SIG DID NOT WORK!!!\n");
			return; 
		}
	}
	//DWORD_PTR addr = (DWORD_PTR)GetProcAddress(h_shell32,"DllGetClassObject") + 0x85;
	addr = addr + 4 + *(DWORD*)addr;
	PSHELLGUIDS table = (PSHELLGUIDS)addr;

	dbgprintf(L"Got table at %p",table);
	while ( &table->rclsid )
	{
		if ( table->rclsid == CLSID_StartMenuPin )
		{
			DWORD old;
			VirtualProtect(table,sizeof(SHELLGUIDS),PAGE_EXECUTE_READWRITE,&old);
			CreateStartMenuPinInstance = table->CreateFunc;
			dbgprintf(L"CreateStartMenuPinInstance = %p",CreateStartMenuPinInstance);
			table->CreateFunc = NewCreateStartMenuPinInstance;
			break;
		}
		table++;
	}
}

