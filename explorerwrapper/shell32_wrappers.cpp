#include "framework.h"
#include "autoplay.h"
#include "dbgprint.h"
#include "version.h"
#include "shell32_wrappers.h"
#include "augmentedshellfolder.h"
#include "MinHook.h"
#include "knownfolders.h"
#include "registry.h"

DWORD bEnableUWPAppsInStart = true;

typedef PVOID (WINAPI *ResolveDelayLoadedAPIAPI)(PVOID ParentModuleBase, PVOID DelayloadDescriptor, PVOID FailureDllHook, PVOID FailureSystemHook,PIMAGE_THUNK_DATA ThunkAddress,ULONG Flags);
static ResolveDelayLoadedAPIAPI ResolveDelayLoadedAPI;
static PVOID CoCreateInstanceBase;
static PVOID SHGetValueWSHCore;

//remove pintostart verb
LSTATUS WINAPI SHGetValueNEW(
  _In_         HKEY hkey,
  _In_opt_     LPCWSTR pszSubKey,
  _In_opt_     LPCWSTR pszValue,
  _Out_opt_    LPDWORD pdwType,
  _Out_opt_    LPVOID pvData,
  _Inout_opt_  LPDWORD pcbData
)
{	
	WCHAR buf[100];
	DWORD bufsize = sizeof(buf);
	if ( lstrcmp(pszValue,L"LegacyDisable") == 0 )
	if ( (RegQueryValueEx(hkey,L"MUIVerb",NULL,NULL,(LPBYTE)buf,&bufsize) == ERROR_SUCCESS) && (lstrcmpi(buf,L"@shell32.dll,-51201") == 0) ) return ERROR_SUCCESS;
	return SHGetValueW(hkey,pszSubKey,pszValue,pdwType,pvData,pcbData);
}

bool(__fastcall* IsSearchEnabled)();
extern "C" bool WINAPI IsSearchEnabledNEW()
{
	//dbgprintf(L"IsSearchEnabledNEW\n");
	return 1;
}

PVOID WINAPI ResolveDelayLoadedAPINEW(PVOID ParentModuleBase, PVOID DelayloadDescriptor, PVOID FailureDllHook, PVOID FailureSystemHook,PIMAGE_THUNK_DATA ThunkAddress,ULONG Flags)
{
	dbgprintf(L"ResolveDelayLoadedAPINEW\n");
	PVOID retfunc = ResolveDelayLoadedAPI(ParentModuleBase,DelayloadDescriptor,FailureDllHook,FailureSystemHook,ThunkAddress,Flags);
	if (retfunc == CoCreateInstanceBase)
	{
		retfunc = Shell32_CoCreateInstance;
		ThunkAddress->u1.Function = (DWORD_PTR)retfunc;
	}
	if (retfunc == SHGetValueWSHCore)
	{
		retfunc = SHGetValueNEW;
		ThunkAddress->u1.Function = (DWORD_PTR)retfunc;
	}
	return retfunc;
}

BOOL __stdcall ILIsEqualNEW(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	dbgprintf(L"ILIsEqualNEW\n");
	IShellFolder* ppshf = 0;
	HRESULT v4 = SHGetDesktopFolder(&ppshf);
	if (v4 >= 0)
	{
		v4 = ppshf->CompareIDs(0x10000000i64, pidl1, pidl2);
		ppshf->Release();
	}
	return v4 == 0;
}

HRESULT __stdcall SHEvaluateSystemCommandTemplateNEW(PCWSTR pszCmdTemplate, PWSTR* ppszApplication, PWSTR* ppszCommandLine, PWSTR* ppszParameters)
{
	dbgprintf(L"SHEvaluateSystemCommandTemplateNEW\n");
	return S_OK;
	//return SHEvaluateSystemCommandTemplateWithOptions((unsigned __int16*)pszCmdTemplate, ppszParameters);
}

HRESULT(__stdcall* Shell32_DllGetClassObject)(REFCLSID rclsid, const IID* const riid, LPVOID* ppv);
HRESULT __stdcall Shell32_DllGetClassObject_Hook(REFCLSID rclsid, const IID* const riid, LPVOID* ppv)
{
	HRESULT result = Shell32_DllGetClassObject(rclsid, riid, ppv);
	if (result != S_OK && (rclsid == CLSID_ProgramsFolderAndFastItems || rclsid == CLSID_ProgramsFolder || rclsid == CLSID_StartMenuFolder))
	{
		*ppv = new CProgramsFolderClassFactory(rclsid);
		return S_OK;
	}

	return result;
}

void HookShell32()
{
	dbgprintf(L"1\n");
	ResolveDelayLoadedAPI = (ResolveDelayLoadedAPIAPI)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"ResolveDelayLoadedAPI");
	ChangeImportedAddress(GetModuleHandle(L"shell32.dll"), "API-MS-WIN-CORE-DELAYLOAD-L1-1-1.DLL", ResolveDelayLoadedAPI, ResolveDelayLoadedAPINEW);
	//ResolveDelayLoadedAPI = (ResolveDelayLoadedAPIAPI)GetProcAddress(GetModuleHandle(L"api-ms-win-core-delayload-l1-1-1.dll"),"ResolveDelayLoadedAPI");
	dbgprintf(L"%i\n",(unsigned long long)ResolveDelayLoadedAPI);
	dbgprintf(L"2\n");
	CoCreateInstanceBase = GetProcAddress(GetModuleHandle(L"combase.dll"),"CoCreateInstance");
	Shell32_DllGetClassObject = (decltype(Shell32_DllGetClassObject))GetProcAddress(GetModuleHandle(L"shell32.dll"),"DllGetClassObject");
	MH_CreateHook(Shell32_DllGetClassObject, Shell32_DllGetClassObject_Hook,(LPVOID*)&Shell32_DllGetClassObject);
	dbgprintf(L"3\n");

	SHGetValueWSHCore = GetProcAddress(LoadLibrary(L"shcore.dll"),"SHGetValueW");

	dbgprintf(L"5\n");
	//auto ordinal902 = GetProcAddress(LoadLibrary(L"shell32.dll"),(LPSTR)902);
	//ChangeImportedAddress(LoadLibrary(L"shell32.dll"),"shlwapi.DLL", GetProcAddress(LoadLibrary(L"shlwapi.dll"), "SHAboutInfo"), SHAboutInfoWNEW);
	ChangeImportedAddress(GetModuleHandle(0),"shell32.DLL", GetProcAddress(LoadLibrary(L"shell32.DLL"), (LPSTR)902), IsSearchEnabledNEW);

	//ChangeImportedAddress(GetModuleHandle(0),"shell32.DLL", GetProcAddress(LoadLibrary(L"shell32.DLL"), (LPSTR)719), SHParseDarwinIDFromCacheWNew);

	//todo: evaluate if this is needed
	ChangeImportedAddress(GetModuleHandle(0),"shell32.DLL", GetProcAddress(LoadLibrary(L"shell32.DLL"), "ILIsEqual"), ILIsEqualNEW);

	uintptr_t Win32PinCheck = FindPattern((uintptr_t)LoadLibrary(L"shell32.dll"), "41 8B E9 49 8B F0 48 8B DA 48 8B F9 48 8D 0D ?? ?? ?? ?? E8");
	if (Win32PinCheck && g_osVersion.BuildNumber() >= 19045)
	{
		dbgprintf(L"Win32PinCheck %i", Win32PinCheck);
		Win32PinCheck += 19;
		uint8_t* bytes = (uint8_t*)(Win32PinCheck + 5 + *reinterpret_cast<int32_t*>(Win32PinCheck + 1));
		DWORD old;
		VirtualProtect(bytes, 3, PAGE_EXECUTE_READWRITE, &old);
		bytes[0] = 0xB0;
		bytes[1] = 0x00;
		bytes[2] = 0xC3;
		VirtualProtect(bytes, 3, old, 0);
	}
	
	dbgprintf(L"6\n");
	g_registry.QueryValue(L"EnableUWPAppsInStart",(LPBYTE)&bEnableUWPAppsInStart,sizeof(DWORD));
}

HRESULT BindToDesktop(LPCITEMIDLIST pidl, IShellFolder** ppsfResult)
{
	HRESULT hr;
	IShellFolder* psfDesktop;

	*ppsfResult = NULL;

	hr = SHGetDesktopFolder(&psfDesktop);
	if (FAILED(hr))
		return hr;

	hr = psfDesktop->BindToObject(pidl, NULL, IID_PPV_ARGS(ppsfResult));

	return hr;
}

enum
{
	ASFF_DEFAULT = 0x00000000,
	ASFF_SORTDOWN = 0x00000001,
	ASFF_MERGESAMEGUID = 0x00000002,
	ASFF_COMMON = 0x00000004,
	ASFF_UNK = 0x0000000A,

	ASFF_DEFNAMESPACE_BINDSTG = 0x00000100,
	ASFF_DEFNAMESPACE_COMPARE = 0x00000200,
	ASFF_DEFNAMESPACE_VIEWOBJ = 0x00000400,
	ASFF_DEFNAMESPACE_ATTRIB = 0x00001800,
	ASFF_DEFNAMESPACE_DISPLAYNAME = 0x00001000,
	ASFF_DEFNAMESPACE_UIOBJ = 0x00002000,
	ASFF_DEFNAMESPACE_ITEMDATA = 0x00004000,
	ASFF_DEFNAMESPACE_ALL = 0x0000FF00
};

struct MERGEDFOLDERINFO
{
	UINT csidl;
	UINT uANSFlags;
	LPCGUID pguidObj;
	UINT idk;
};

const MERGEDFOLDERINFO c_rgmfiProgramsFolderAndFastItems[] = {
	{   CSIDL_PROGRAMS | CSIDL_FLAG_CREATE,    ASFF_DEFNAMESPACE_ALL | ASFF_UNK,                 &CLSID_ProgramsFolder, 2},
	{   CSIDL_COMMON_PROGRAMS,                 ASFF_COMMON | ASFF_MERGESAMEGUID | ASFF_UNK,                 &CLSID_ProgramsFolderCommon, 2},
	{   CSIDL_STARTMENU | CSIDL_FLAG_CREATE,     ASFF_UNK, &CLSID_StartMenu, 1 },
	{   CSIDL_COMMON_STARTMENU,                  ASFF_UNK, &CLSID_StartMenu, 1 },
};

const MERGEDFOLDERINFO c_rgmfiStartMenu[] = {
	{   CSIDL_STARTMENU | CSIDL_FLAG_CREATE,    ASFF_DEFNAMESPACE_ALL,  &CLSID_StartMenu,1 },
	{   CSIDL_COMMON_STARTMENU,                 ASFF_COMMON,            &CLSID_StartMenu,1 },
};

const MERGEDFOLDERINFO c_rgmfiProgramsFolder[] = {
	{   CSIDL_PROGRAMS | CSIDL_FLAG_CREATE,    ASFF_DEFNAMESPACE_ALL | ASFF_MERGESAMEGUID,                 &CLSID_ProgramsFolder, 2},
	{   CSIDL_COMMON_PROGRAMS,                 ASFF_COMMON | ASFF_MERGESAMEGUID,                 &CLSID_ProgramsFolderCommon, 2}
};

struct FolderDefinition //reversed from 1507
{
	const GUID* FolderID;
	DWORD flag;
	DWORD flag2;
};

const FolderDefinition c_rgrfi[] = 
{   
	{&FOLDERID_ApplicationShortcuts , 0, 0x38},
	{&FOLDERID_StartMenu , 0, 0x18},
	{&FOLDERID_Programs , 0, 0x14},
	{&FOLDERID_CommonStartMenu , 0, 0x1A},
	{&FOLDERID_CommonPrograms , 0, 0x16},
	//{&FOLDERID_Games , 0, 0x10}, //dont include games folder till further notice, causes duplicate entries
	{&FOLDERID_Desktop , 0x1000, 1},
	{&FOLDERID_PublicDesktop , 0x1000, 3},
	{&FOLDERID_UserPinned , 0, 0x20},
	{&FOLDERID_CommonStartMenuPlaces , 0, 0x61} 
};


//adapted from reactos
static HRESULT GetMergedFolders(const MERGEDFOLDERINFO* Folders, int length, IShellFolder** ppsfStartMenu)
{
	HRESULT hr;
	IAugmentedShellFolder* pasf = nullptr;

	*ppsfStartMenu = NULL;
	hr = CoCreateInstance(CLSID_MergedFolder, 0LL, 1u, IID_IAugmentedFolder, (LPVOID*)&pasf);

	if (bEnableUWPAppsInStart)
	{
		IShellItem* shellItem;
		IShellFolder* AppsFolder;
		SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DONT_VERIFY, 0LL, IID_PPV_ARGS(&shellItem));
		if (shellItem)
		{
			shellItem->BindToHandler(0, BHID_SFObject, IID_PPV_ARGS(&AppsFolder));

			pasf->AddNameSpace((LPGUID)&FOLDERID_AppsFolder, AppsFolder, 0, ASFF_DEFNAMESPACE_ALL | ASFF_UNK, 2);
			AppsFolder->Release();
			shellItem->Release();
		}

		for (int i = 0; i < _ARRAYSIZE(c_rgrfi); ++i)
		{
			auto& Def = c_rgrfi[i];
			if (Def.FolderID != &FOLDERID_ApplicationShortcuts && (Def.flag2 & 0x10) != 0)
			{
				SHGetKnownFolderItem(*Def.FolderID, KF_FLAG_DONT_VERIFY, 0LL, IID_PPV_ARGS(&shellItem));
				if (shellItem)
				{
					shellItem->BindToHandler(0, BHID_SFObject, IID_PPV_ARGS(&AppsFolder));

					pasf->AddNameSpace((LPGUID)Def.FolderID, AppsFolder, 0, 0xFF08 | ASFF_DEFNAMESPACE_ALL | ASFF_UNK, 1);
					AppsFolder->Release();
				}
			}
		}

		shellItem->Release();
	}
	else
	{
		for (int i = 0; i < length; ++i)
		{
			auto Info = Folders[i];
			LPITEMIDLIST pidlUserStartMenu;
			IShellFolder* psfUserStartMenu = nullptr;

			hr = SHGetSpecialFolderLocation(NULL, Info.csidl, &pidlUserStartMenu);
			if (FAILED(hr)) continue;

			hr = BindToDesktop(pidlUserStartMenu, &psfUserStartMenu);
			if (FAILED(hr))
			{
				ILFree(pidlUserStartMenu);
				continue;
			}

			hr = pasf->AddNameSpace((LPGUID)Info.pguidObj, psfUserStartMenu, pidlUserStartMenu, Info.uANSFlags, Info.idk);
			if (FAILED(hr))
			{
				psfUserStartMenu->Release();
				ILFree(pidlUserStartMenu);
				continue;
			}
			psfUserStartMenu->Release();
			ILFree(pidlUserStartMenu);
		}
	}
	*ppsfStartMenu = pasf;
	return hr;
}

HRESULT WINAPI Shell32_CoCreateInstance(
	__in   REFCLSID rclsid,
	__in   LPUNKNOWN pUnkOuter,
	__in   DWORD dwClsContext,
	__in   REFIID riid,
	__out  LPVOID* ppv
)
{
	HRESULT result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (rclsid == CLSID_ProgramsFolderAndFastItems && result != S_OK)
	{
		IShellFolder* ShellFolder;
		HRESULT result = GetMergedFolders(c_rgmfiProgramsFolderAndFastItems, _ARRAYSIZE(c_rgmfiProgramsFolderAndFastItems), &ShellFolder);
		result = ShellFolder->QueryInterface(riid, ppv);
		ShellFolder->Release();
		return result;
	}

	
	if (result == S_OK && rclsid == CLSID_AutoPlayUI)
	{
		*ppv = new CAutoPlayWrapper((IAutoPlayUI*)*ppv);
	}

	return result;
}

CProgramsFolderClassFactory::CProgramsFolderClassFactory(REFCLSID clsid)
{
	this->clsid = clsid;
}

CProgramsFolderClassFactory::~CProgramsFolderClassFactory()
{
}

HRESULT __stdcall CProgramsFolderClassFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IClassFactory)
	{
		*ppvObject = static_cast<IClassFactory*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall CProgramsFolderClassFactory::AddRef(void)
{
	return 1;
}

ULONG __stdcall CProgramsFolderClassFactory::Release(void)
{
	return 1;
}

HRESULT __stdcall CProgramsFolderClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
{
	if (pUnkOuter) return CLASS_E_NOAGGREGATION;
	if (this->clsid == CLSID_ProgramsFolderAndFastItems)
	{
		IShellFolder* ShellFolder;
		HRESULT result = GetMergedFolders(c_rgmfiProgramsFolderAndFastItems,_ARRAYSIZE(c_rgmfiProgramsFolderAndFastItems), &ShellFolder);
		result = ShellFolder->QueryInterface(riid, ppvObject);
		ShellFolder->Release();
		return result;
	}
	else if (this->clsid == CLSID_ProgramsFolder)
	{
		IShellFolder* ShellFolder;
		HRESULT result = GetMergedFolders(c_rgmfiProgramsFolder, _ARRAYSIZE(c_rgmfiProgramsFolder), &ShellFolder);
		result = ShellFolder->QueryInterface(riid, ppvObject);
		ShellFolder->Release();
		return result;
	}
	else if (this->clsid == CLSID_StartMenuFolder)
	{
		IShellFolder* ShellFolder;
		HRESULT result = GetMergedFolders(c_rgmfiStartMenu, _ARRAYSIZE(c_rgmfiStartMenu), &ShellFolder);
		result = ShellFolder->QueryInterface(riid, ppvObject);
		ShellFolder->Release();
		return result;
	}
	return E_NOTIMPL;
}

HRESULT __stdcall CProgramsFolderClassFactory::LockServer(BOOL fLock)
{
	return S_OK;
}

