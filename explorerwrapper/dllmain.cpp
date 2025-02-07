#define INITGUID
#define PRERELEASE_COPY
#include "util.h"
#include "common.h"
#include "forwards.h"
#include "StartMenuResolver.h"
#include "TrayObject.h"
#include "dbgprint.h"
#include "ImmersiveShell.h"
#include "TrayNotify.h"
#include "AuthUI.h"
#include "StartMenuPin.h"
#include "ImmersiveFactory.h"
#include "ProjectionFactory.h"
#include "OSVersion.h"
#include "PinnedList.h"
#include "DestinationList.h"
#include "resource.h"
#include "ThemeManager.h"
#include "MinHook.h"
#include "ShellTaskScheduler.h"
#include "RegistryManager.h"
#include "NscTree.h"
#include "RegTreeOptions.h"
#include "shellapi.h"
#include "AutoPlay.h"
#include "StartMenuItemFilter.h"
#include "shell32_wrappers.h"
#include "ShellURL.h"
#include "OptionConfig.h"
#include "AddressImports.h"
#include "PatternImports.h"
#include "MinhookImports.h"
#include "TypeDefinitions.h"

LRESULT CALLBACK NewTrayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//if (s_EnableImmersiveShellStack == 1)
		//SetProgmanAsShell(); // misha: TODO hack

	if (uMsg == 0x56D) return 0;
	if (uMsg == ThemeChangeMessage) //reinit thememanager on themechanged, so that inactive msstyles is updated
	{
		for (int i = 0; i < themeHandles->size; ++i)
		{
			CloseThemeData(themeHandles->data[i]);
		}
		realloc(themeHandles->data, 0);
		themeHandles->size = 0;

		ThemeManagerInitialize();
		EnumWindows(RefreshWindows, (LPARAM)hwnd);

		uMsg = WM_THEMECHANGED;
		return CallWindowProc(g_prevTrayProc, hwnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_DISPLAYCHANGE || uMsg == WM_WINDOWPOSCHANGED)
	{
		RemoveProp(hwnd, L"TaskbarMonitor");
		SetProp(hwnd, L"TaskbarMonitor", (HANDLE)MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY));
		//send displaychanged to desktop
		if (uMsg == WM_DISPLAYCHANGE) PostMessage(hwnd_desktop, 0x44B, 0, 0);
	}

	if (uMsg == 0x574) //handledelayboot
	{
		if (lParam == 3)
			return CallWindowProc(g_prevTrayProc, hwnd, 0x5B5, wParam, lParam); //fire ShellDesktopSwitch event
		if (lParam == 1)
			SetEvent(hEvent_DesktopVisible);
		return 0;
	}

	if (uMsg == WM_THEMECHANGED)
	{
		EnsureWindowColorization(); // Ittr: Correct colorization enablement setting for Win10/11
	}

	if (uMsg == WM_SETTINGCHANGE || uMsg == WM_ERASEBKGND || uMsg == WM_WININICHANGE) // Ittr: Fix taskbar colorization for non-legacy
	{
		if ((IsThemeActive() && !s_ClassicTheme && IsCompositionActive() && !s_DisableComposition) && hwnd == GetTaskbarWnd() && s_ColorizationOptions != 0) // Ittr: Only taskbar needs updating now, start menu and new thumbnail algo correct for themselves
		{
			SetWindowCompositionAttribute(hwnd, &GetTrayAccentProperties(false));
		}
	}

	return CallWindowProc(g_prevTrayProc, hwnd, uMsg, wParam, lParam);
}

// Ittr: Subclass the thumbnail so we can update its colorization as needed
LRESULT CALLBACK NewThumbnailProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SETTINGCHANGE || uMsg == WM_ERASEBKGND || uMsg == WM_WININICHANGE) // Ittr: Fix thumbnail colorization for non-legacy
	{
		if ((IsThemeActive() && !s_ClassicTheme && IsCompositionActive() && !s_DisableComposition) && (g_osVersion.BuildNumber() >= 10074 && hwnd == GetThumbnailWnd()) && s_ColorizationOptions != 0) // Ittr: Only taskbar needs updating now, start menu and new thumbnail algo correct for themselves
		{
			SetWindowCompositionAttribute(hwnd, &GetTrayAccentProperties(true));
		}
	}

	return CallWindowProc(g_prevThumbnailProc, hwnd, uMsg, wParam, lParam);
}

void ShimDesktop()
{
	static int InitOnce = FALSE;
	if (InitOnce) return;
	hwnd_desktop = FindWindow(L"Progman", L"Program Manager");
	HWND hwndTray = GetTaskbarWnd();
	HWND hwndThumbnail = GetThumbnailWnd(); // thumbnail hwnd not being present should not stop the shim
	if (!hwnd_desktop || !hwndTray) return;
	InitOnce = TRUE;
	//hook tray
	g_prevTrayProc = (WNDPROC)GetWindowLongPtr(hwndTray, GWLP_WNDPROC);
	g_prevThumbnailProc = (WNDPROC)GetWindowLongPtr(hwndThumbnail, GWLP_WNDPROC);
	SetWindowLongPtr(hwndTray, GWLP_WNDPROC, (LONG_PTR)NewTrayProc);
	SetWindowLongPtr(hwndThumbnail, GWLP_WNDPROC, (LONG_PTR)NewThumbnailProc);
	//set monitor (doh!)
	SetProp(hwndTray, L"TaskbarMonitor", (HANDLE)MonitorFromWindow(hwndTray, MONITOR_DEFAULTTOPRIMARY));
	//init desktop	
	PostMessage(hwnd_desktop, 0x45C, 1, 1); //wallpaper
	PostMessage(hwnd_desktop, 0x45E, 0, 2); //wallpaper host
	PostMessage(hwnd_desktop, 0x45C, 2, 3); //wallpaper & icons
	PostMessage(hwnd_desktop, 0x45B, 0, 0); //final init
	PostMessage(hwnd_desktop, 0x40B, 0, 0); //pins
}

PVOID WINAPI SHCreateDesktopNEW(PVOID p1)
{
	PVOID ret = SHCreateDesktopOrig(p1);
	ShimDesktop();
	return ret;
}

PVOID WINAPI SHDesktopMessageLoopNEW(PVOID p1)
{
	PVOID ret = SHDesktopMessageLoop(p1);
	SHPtrParamAPI SHCloseDesktopHandle;
	SHCloseDesktopHandle = (SHPtrParamAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"),(LPSTR)206);
	SHCloseDesktopHandle(p1);
	return ret;
}

DWORD WINAPI CTray__SyncThreadProc_hook(LPVOID lpParameter)
{
	if (!g_dwTrayThreadId)
	{
		g_dwTrayThreadId = GetCurrentThreadId();
		dbgprintf(L"set g_dwTrayThreadId to %u", g_dwTrayThreadId);
	}

	return CTray__SyncThreadProc_orig(lpParameter);
}

void HookTrayThread(void)
{
	CTray__SyncThreadProc_orig = (LPTHREAD_START_ROUTINE)FindPattern(
		(uintptr_t)GetModuleHandle(NULL),
		"48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 81 EC 00 03 00 00 48 8B"
	);

	if (CTray__SyncThreadProc_orig)
	{
		MH_CreateHook(
			(void*)CTray__SyncThreadProc_orig,
			(void*)CTray__SyncThreadProc_hook,
			(void**)&CTray__SyncThreadProc_orig
		);
	}
}

void GetOrbDPIAndPos(LPWSTR fName)
{
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

	HDC screen = GetDC(NULL);
	double hPixelsPerInch = GetDeviceCaps(screen, LOGPIXELSX);
	double vPixelsPerInch = GetDeviceCaps(screen, LOGPIXELSY);
	ReleaseDC(NULL, screen);
	double dpi = (hPixelsPerInch + vPixelsPerInch) * 0.5;

	if (dpi >= 120)
	{
		if (dpi >= 144)
		{
			if (dpi >= 192)
			{
				if (abd.uEdge == ABE_LEFT || abd.uEdge == ABE_RIGHT) StringCchCopyW(fName, MAX_PATH, L"6808");
				else if (abd.uEdge == ABE_TOP) StringCchCopyW(fName, MAX_PATH, L"6812");
				else StringCchCopyW(fName, MAX_PATH, L"6804");
			}
			else
			{
				if (abd.uEdge == ABE_LEFT || abd.uEdge == ABE_RIGHT) StringCchCopyW(fName, MAX_PATH, L"6807");
				else if (abd.uEdge == ABE_TOP) StringCchCopyW(fName, MAX_PATH, L"6811");
				else StringCchCopyW(fName, MAX_PATH, L"6803");
			}
		}
		else
		{
			if (abd.uEdge == ABE_LEFT || abd.uEdge == ABE_RIGHT) StringCchCopyW(fName, MAX_PATH, L"6806");
			else if (abd.uEdge == ABE_TOP) StringCchCopyW(fName, MAX_PATH, L"6810");
			else StringCchCopyW(fName, MAX_PATH, L"6802");
		}
	}
	else
	{
		if (abd.uEdge == ABE_LEFT || abd.uEdge == ABE_RIGHT) StringCchCopyW(fName, MAX_PATH, L"6805");
		else if (abd.uEdge == ABE_TOP) StringCchCopyW(fName, MAX_PATH, L"6809");
		else StringCchCopyW(fName, MAX_PATH, L"6801");
	}
}

HANDLE __stdcall LoadImageW_CallHook(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad)
{
	dbgprintf(L"LoadImageW_CallHook has been called!");

	WCHAR szExeDir[MAX_PATH];
	GetModuleFileNameW(NULL, szExeDir, MAX_PATH);
	WCHAR* backslash = StrRChrW(szExeDir, NULL, L'\\');
	if (*backslash == L'\\')
		*backslash = L'\0';

	WCHAR szOrbDir[MAX_PATH];
	LSTATUS res = g_registry.QueryValue(L"OrbDirectory", (LPBYTE)szOrbDir, sizeof(szOrbDir));

	if (!*szOrbDir || ERROR_SUCCESS != res)
		return LoadImageW(hInst, name, type, cx, cy, fuLoad);

	WCHAR szOrbFile[MAX_PATH];
	GetOrbDPIAndPos(szOrbFile);

	WCHAR szOrbPath[MAX_PATH * 3];
	wsprintfW(
		szOrbPath,
		L"%s\\orbs\\%s\\%s.bmp",
		szExeDir,
		szOrbDir,
		szOrbFile
	);

	if (FileExists(szOrbPath) == FALSE)
		return LoadImageW(hInst, name, type, cx, cy, fuLoad);
	else
		return LoadImageW(NULL, szOrbPath, IMAGE_BITMAP, 0, 0, fuLoad | LR_LOADFROMFILE);
}

void HookLoadImageForSizeAndFont()
{
	auto callLoadImage = (uintptr_t)FindPattern((uintptr_t)GetModuleHandle(0), "FF 15 ?? ?? ?? ?? 48 89 43 ?? 48 85 C0 74 ?? 4C 8D ?? ?? ?? BA ?? ?? ?? ?? 48 8B C8 FF 15");
	if (callLoadImage)
	{
		//write a nop
		DWORD old;
		VirtualProtect((void*)callLoadImage, 1, PAGE_EXECUTE_READWRITE, &old);
		*reinterpret_cast<char*>(callLoadImage) = 0x90;
		VirtualProtect((void*)callLoadImage, 1, old, 0);

		callLoadImage += 1;

		// write a call to our function
		DetourCall((void*)callLoadImage, LoadImageW_CallHook);
	}

	char* callDrawExtended = (char*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 08 57 48 83 EC 30 33 DB 48 8B F9 48 39 59 40");
	if (!callDrawExtended) return;

	if (callDrawExtended)
	{
		unsigned char bytes[] = { 0xB0,0x01,0xC3 };
		ChangeImportedPattern(callDrawExtended, bytes, sizeof(bytes));
	}
}

void ModifyDesktopHwnd()
{
	uintptr_t desktopHwnd = FindPattern((uintptr_t)GetModuleHandle(0), "74 ?? 48 3B 3D ?? ?? ?? ?? 8D 43 01 0F 45 D8");
	if (desktopHwnd)
	{
		desktopHwnd += 2;
		v_hwndDesktop = (HWND*)(desktopHwnd + 7 + *reinterpret_cast<signed int*>(desktopHwnd + 3));
	}
}

enum BlockHotKeyRegistrationFlags : __int32
{
	BHKRF_None = 0x0,
	BHKRF_Always = 0x1,
	BHKRF_PpiEdition = 0x2,
	BHKRF_AssignedAccessMultiAppMode = 0x4,
	BHKRF_ShellLauncher = 0x8,
};

const struct IMMERSIVE_WINDOW_MESSAGE_SERVICE_HOTKEY_REGISTRATION
{
	BlockHotKeyRegistrationFlags blockFlags;
	int id;
	unsigned int fsModifiers;
	unsigned int vk;
};

// experimental hotkey fix 1 - broke uwp
HRESULT(__fastcall* CImmersiveWindowMessageService__RequestHotkeys)(void* a1, unsigned int a2, IMMERSIVE_WINDOW_MESSAGE_SERVICE_HOTKEY_REGISTRATION* a3, void* a4, unsigned int* a5);
HRESULT CImmersiveWindowMessageService__RequestHotkeys_Hook(void* a1, unsigned int a2, IMMERSIVE_WINDOW_MESSAGE_SERVICE_HOTKEY_REGISTRATION* a3, void* a4, unsigned int* a5)
{
	dbgprintf(L"CImmersiveWindowMessageService__RequestHotkeys");
	if (a3->vk == VK_LWIN || a3->vk == VK_RWIN || (a3->vk == VK_ESCAPE && a3->fsModifiers & VK_CONTROL)) // fix win key
	{
		dbgprintf(L"FIXING WINDOWS KEY");
		return S_OK;
	}

	return CImmersiveWindowMessageService__RequestHotkeys(a1,a2,a3,a4,a5);
}

// experimental hotkey fix 2 - doesn't work at present
UINT shellHook = 0;

UINT(WINAPI* fRegisterWindowMessageW)(LPCWSTR lpString);
UINT WINAPI RegisterWindowMessageWNEW(LPCWSTR lpString)
{
	dbgprintf(L"RegisterWindowMessageWNEW %s",lpString);
	if (wcscmp(L"SHELLHOOK", lpString) == 0)
	{
		dbgprintf(L"RegisterWindowMessageWNEW REDIRD");

		if (shellHook != 0)
			return shellHook;

		shellHook = fRegisterWindowMessageW(L"SHELLHOOK");
		return shellHook;
	}
	return fRegisterWindowMessageW(lpString);
}

// experimental hotkey fix 3 - buggy results but *does* appear to work
HRESULT(__fastcall* CTaskBand_HandleShellHook)(PVOID ctaskband, int id, HWND a3);

HRESULT(__fastcall* OnShellHookMessage)(void* a1);
HRESULT OnShellHookMessage_Hook(void* a1) //gets called when start menu is to be opened - a bit temperamental
{
	// key to note: at the moment, we can either do this for bugged start menu behaviour, or we can return S_OK and have no menu on the hotkey at all.
	// neither is ideal, but we can probably ship m2 like this and fix properly later

	if (CTaskBandPtr)
		return CTaskBand_HandleShellHook(CTaskBandPtr,7,0);

	return OnShellHookMessage(a1);
}

void HookShell32();
void HookAPIs() // largely a legacy function now
{
	// 24H2+ - W32PTP
	if (g_osVersion.BuildNumber() >= 26100)
	{
		HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

		if (twinui_pcshell)
		{
			CTaskbandPin_CreateInstance = (CTaskbandPin_CreateInstance_t)FindPattern((uintptr_t)twinui_pcshell, "40 53 48 83 EC 20 48 8B D9 48 8D 15 ?? ?? ?? ?? B9 80 00 00 00 E8 ?? ?? ?? ?? 48 85 C0");
		}
	}

	// Change and fix core desktop components
	hEvent_DesktopVisible = CreateEvent(NULL, TRUE, FALSE, L"ShellDesktopVisibleEvent");
	SHCreateDesktopOrig = (SHCreateDesktopAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"), (LPSTR)200);
	ChangeImportedAddress(GetModuleHandle(NULL), "shell32.dll", SHCreateDesktopOrig, SHCreateDesktopNEW);
	SHDesktopMessageLoop = (SHCreateDesktopAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"), (LPSTR)201);
	ChangeImportedAddress(GetModuleHandle(NULL), "shell32.dll", SHDesktopMessageLoop, SHDesktopMessageLoopNEW);

	// ???
	ModifyDesktopHwnd();

	// We run the Minhook patches here
	ChangeMinhookImports();

	//CImmersiveWindowMessageService__RequestHotkeys = (decltype(CImmersiveWindowMessageService__RequestHotkeys))FindPattern((uintptr_t)LoadLibrary(L"twinui.dll"), "4C 8B DC 4D 89 43 ?? 57 41 54 41 55 41 56 41 57 48 83 EC");
	//MH_CreateHook(static_cast<LPVOID>(CImmersiveWindowMessageService__RequestHotkeys), CImmersiveWindowMessageService__RequestHotkeys_Hook, reinterpret_cast<LPVOID*>(&CImmersiveWindowMessageService__RequestHotkeys));

	//fRegisterWindowMessageW = (decltype(fRegisterWindowMessageW))GetProcAddress(LoadLibraryW(L"user32.dll"),"RegisterWindowMessageW");

	// disabled - <1607 doesnt like atm + unfinished. sorry! uncomment if you're testing
	//CTaskBand_HandleShellHook = (decltype(CTaskBand_HandleShellHook))FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 08 55 56 57 41 54 41 55 48 83 EC ?? 83 FA 07");
	//OnShellHookMessage = (decltype(OnShellHookMessage))FindPattern((uintptr_t)LoadLibraryW(L"twinui.pcshell.dll"), "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 48 8B 40 ?? FF 15 ?? ?? ?? ?? 84 C0 0F 85 ?? ?? ?? ?? 38 83");

	// disabled - <1607 doesnt like atm + unfinished. sorry! uncomment if you're testing
	//MH_CreateHook(static_cast<LPVOID>(OnShellHookMessage), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&OnShellHookMessage));

	// Prevent theme overrides applying to file explorer *VERY IMPORTANT*
	HookTrayThread();

	// 1. shell32.dll - hack created startmenupin instance
	// 2. shell32.dll - patch delayload stuff
	StartMenuPin_PatchShell32();
	HookShell32();

	// Handle custom start orb feature
	HookLoadImageForSizeAndFont();

	// Enable MinHook hooks at the end
	MH_EnableHook(MH_ALL_HOOKS);
}

BOOL WINAPI GetUserObjectInformationNew(HANDLE hObj, int nIndex, PVOID pvInfo, DWORD nLength, LPDWORD lpnLengthNeeded)
{
	lstrcpy(LPWSTR(pvInfo), L"Winlogon");
	return TRUE;
}

BOOL WINAPI GetWindowBandNew(HWND hwnd, DWORD* out)
{
	BOOL ret = GetWindowBandOrig(hwnd, out);
	DWORD origband = (DWORD)GetProp(GetAncestor(hwnd, GA_ROOTOWNER), L"explorer7.WindowBand");
	//dbgprintf(L"GetWindowBand %p %p %p",hwnd,*out,origband);
	if (origband && out) *out = origband;
	return ret;
}

UINT_PTR WINAPI SetTimer_WUI(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	if (nIDEvent == 0x2252CE37)
		ShowWindow(hWnd, SW_HIDE);
	return SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
}

// Used even when immersive UI is not active in some cases..?
void HookImmersive()
{
	HMODULE immersiveui = LoadLibrary(L"Windows.UI.Immersive.dll");
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	CreateWindowInBandOrig = (CreateWindowInBandAPI)GetProcAddress(hUser32, "CreateWindowInBand");

	if (s_EnableImmersiveShellStack == 1)
		CreateWindowInBandExOrig = (CreateWindowInBandExAPI)GetProcAddress(hUser32, "CreateWindowInBand");

	GetWindowBandOrig = (GetWindowBandAPI)GetProcAddress(hUser32, "GetWindowBand");
	ChangeImportedAddress(immersiveui, "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	ChangeImportedAddress(immersiveui, "user32.dll", GetWindowBandOrig, GetWindowBandNew);
	ChangeImportedAddress(immersiveui, "user32.dll", GetUserObjectInformation, GetUserObjectInformationNew);
	ChangeImportedAddress(immersiveui, "user32.dll", SetTimer, SetTimer_WUI);

	if (!s_EnableImmersiveShellStack || g_osVersion.BuildNumber() < 10074) // Ittr: If user *either* has UWP disabled, or they are NOT on Windows 10, run legacy window band code
	{
		//bugbug!!!
		ChangeImportedAddress(GetModuleHandle(L"twinui.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
		ChangeImportedAddress(GetModuleHandle(L"authui.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
		ChangeImportedAddress(GetModuleHandle(L"shell32.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);

		ChangeImportedAddress(GetModuleHandle(L"twinapi.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
		ChangeImportedAddress(GetModuleHandle(L"Windows.UI.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	}
}

FARPROC
WINAPI
GetProcAddress_Hook(
	HMODULE hModule,
	LPCSTR lpProcName
)
{
	//dbgprintf(L"GetProcAddress Hook\n");
	return GetProcAddress(hModule, lpProcName);
}

// Basically this allows explorer to actually work on builds >9200
void PatchShunimpl()
{
	uintptr_t shunImpl = (uintptr_t)GetModuleHandle(L"shunimpl.dll");
	if (!shunImpl) return;
	char* dllmainSHUNIMPL = (char*)FindPattern(shunImpl, "48 83 EC 28 83 FA 01");

	if (dllmainSHUNIMPL)
	{
		unsigned char bytes[] = { 0xB0,0x01,0xC3 };
		ChangeImportedPattern(dllmainSHUNIMPL, bytes, sizeof(bytes));
	}
}

// Where we need to close explorer silently (such as to block people from using awful, horrendous software...)
void ExitExplorerSilently()
{
	// we do these blocks of code like this, so that the 0xc0000142 error doesn't appear
	LPDWORD exitCode;
	GetExitCodeProcess(L"explorer.exe", exitCode);
	ExitProcess((UINT)exitCode); // exit explorer
}

// Initialize the inactive theme engine
void ThemeHandlesInit()
{
	themeHandles = new wiktorArray<HTHEME>();
	themeHandles->data = 0;
	themeHandles->size = 0;
}

// Terminate inactive theme engine when needed
void EndThemeHandles()
{
	realloc(themeHandles->data, 0);
	themeHandles->size = 0;
	delete themeHandles;
}

// WINDOWS 11
void InitPinnedListHack()
{
	// == CPINNEDLIST HACK ==

	HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

	// CTaskbandPin_CreateInstance
	if (twinui_pcshell)
	{
		// Method 1: Direct function preamble (dangerous; breaks if they modify the fields of CTaskbandPin or its superclass(es))
		// 40 53 48 83 EC 20 48 8B D9 48 8D 15 ?? ?? ?? ?? B9 80 00 00 00 E8 ?? ?? ?? ?? 48 85 C0
		/*matchCTaskbandPinCreateInstance = (PBYTE)FindPattern(
			pFile,
			dwSize,
			"\x40\x53\x48\x83\xEC\x20\x48\x8B\xD9\x48\x8D\x15\x00\x00\x00\x00\xB9\x80\x00\x00\x00\xE8\x00\x00\x00\x00\x48\x85\xC0",
			"xxxxxxxxxxxx????xxxxxx????xxx",
			&numMatchesCTaskbandPinCreateInstance
		);*/

		// Method 2: winrt::Windows::Internal::Shell::implementation::PinManager::IsItemPinned
		// 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 83 64 24 ?? ?? 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 8B 8D ?? ?? ?? ?? 85 C0
		//                                                                   ^^^^^^^^^^^
		PBYTE matchCTaskbandPinCreateInstance = (PBYTE)FindPattern((uintptr_t)twinui_pcshell, "48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 83 64 24 ?? ?? 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 8B 8D ?? ?? ?? ?? 85 C0");

		if (matchCTaskbandPinCreateInstance)
		{
			matchCTaskbandPinCreateInstance += 21;
			matchCTaskbandPinCreateInstance += 5 + *(int*)(matchCTaskbandPinCreateInstance + 1);

		}

		if (!matchCTaskbandPinCreateInstance)
		{
			// wil::out_param() destructor inlined
			// 0F 1F 44 00 00 48 83 64 24 ?? ?? 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 8B 8D ?? ?? ?? ?? 85 C0
			//                                                    ^^^^^^^^^^^
			matchCTaskbandPinCreateInstance = (PBYTE)FindPattern((uintptr_t)twinui_pcshell, "0F 1F 44 00 00 48 83 64 24 ?? ?? 48 8D 4C 24 ?? E8 ?? ?? ?? ?? 48 8B 8D ?? ?? ?? ?? 85 C0");

			if (matchCTaskbandPinCreateInstance)
			{
				matchCTaskbandPinCreateInstance += 16;
				matchCTaskbandPinCreateInstance += 5 + *(int*)(matchCTaskbandPinCreateInstance + 1);
			}
		}

		if (matchCTaskbandPinCreateInstance)
		{
			CTaskbandPin_CreateInstance = (CTaskbandPin_CreateInstance_t)matchCTaskbandPinCreateInstance;
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	// Ittr: We initialise values for closing program if incompatible software is present
	WCHAR programPath[MAX_PATH] = L"\\Stardock\\WindowBlinds 11\\unins000.exe";
	WCHAR blacklistPath[MAX_PATH];
	ExpandEnvironmentStringsW(L"%ProgramFiles%", (LPWSTR)blacklistPath, sizeof(blacklistPath));
	lstrcat(blacklistPath, programPath);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		PatchShunimpl();

		if (GetFileAttributesW((LPCWSTR)blacklistPath) != INVALID_FILE_ATTRIBUTES) // Windowblinds blockage part 1 - create user-facing error
			CrashError(); // The user-facing crash message - we do these blocks of code like this, so that the 0xc0000142 error doesn't appear

		if (g_osVersion.BuildNumber() >= 26100)
		{
			InitPinnedListHack();
		}

		CreateShellFolder(); // Fix shell folder for 1607+...
		EnsureWindowColorization(); // Correct colorization enablement setting for Win10/11
		FirstRunCompatibilityWarning(); // Warn users on Windows 11 24H2+ and Server 2022 of potential problems
		FirstRunPrereleaseWarning(); // Warn users if this is a pre-release build that this is the case on first run ONLY
		ThemeHandlesInit(); // Basically start the inactive theme management process

		dbgprintf(L"Dll Attach\n");

		// Ittr: Load user configuration from the registry
		// - Important that we do this first before applying API hooks
		InitializeConfiguration();

		// Ittr: Handle pattern byte replacement patches, usually for disabling or fixing features
		ChangePatternImports();

		// Ittr: Handle address import changes, usually for rewriting or modifying results from API
		ChangeAddressImports();

		g_hInstance = hModule;
		if (GetModuleHandle(L"DisplaySwitch.exe"))
		{
			dbgprintf(L"loaded into displayswitch %p %s!", GetCurrentProcessId(), GetCommandLine());
			HookImmersive();
		}
		else
		{
			HookAPIs();
		}
	}
	break;
	case DLL_THREAD_ATTACH:
	{
		if (!g_alttabhooked && GetModuleHandle(L"alttab.dll"))
		{
			CreateWindowInBandOrig = (CreateWindowInBandAPI)GetProcAddress(GetModuleHandle(L"user32.dll"), "CreateWindowInBand");
			ChangeImportedAddress(GetModuleHandle(L"alttab.dll"), "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
			g_alttabhooked = TRUE;
		}

		if (GetFileAttributes((LPCWSTR)blacklistPath) != INVALID_FILE_ATTRIBUTES) // Windowblinds blockage part 2 - actually stops the program from running
			ExitExplorerSilently(); //byebye WB users

	}
	break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		EndThemeHandles();
		break;
	}
	return TRUE;
}

extern "C" HRESULT WINAPI Explorer_CoCreateInstance(
	__in   REFCLSID rclsid,
	__in   LPUNKNOWN pUnkOuter,
	__in   DWORD dwClsContext,
	__in   REFIID riid,
	__out  LPVOID* ppv
)
{
	HRESULT result;
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);

	if (rclsid == CLSID_PersonalStartMenu && riid == IID_IShellItemFilter && result != S_OK && g_osVersion.BuildNumber() >= 10074) //Ittr: as far as im aware doesnt cause crashing on 1507/11. needs further checking when im awake
	{
		auto shellItemFilter = new CStartMenuItemFilter();
		result = shellItemFilter->QueryInterface(riid, ppv);
	}

	if (rclsid == CLSID_SysTray) //create Metro before tray
	{
		dbgprintf(L"create Metro before tray\n");
		HookImmersive();

		if (s_EnableImmersiveShellStack == 1) // Ittr: Only create TWinUI UWP mode here if we are going to use it
			CreateTwinUI_UWP();

	}
	if (rclsid == CLSID_RegTreeOptions && riid == IID_IRegTreeOptions7) //upgrading RegTreeOptions interface
	{
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IRegTreeOptions8, ppv);
		*ppv = new CRegTreeOptionsWrapper((IRegTreeOptions8*)*ppv);
	}

	if (riid == IID_IAuthUILogonSound7 && result != S_OK)
	{
		dbgprintf(L"Wrap authuilogonsound7\n");
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IAuthUILogonSound10, ppv);
	}

	if (rclsid == CLSID_UserAssist && result != S_OK)
	{
		if (riid == IID_IUserAssist7)
			result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IUserAssist10, ppv);
		else if (riid == IID_IUserAssist72)
			result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IUserAssist102, ppv);
		else
		{
			dbgprintf(L"Warning, unknown useraassist riid!!!!!");
			dbgprintf(L"Warning, unknown useraassist riid!!!!!");
		}
	}

	if (rclsid == CLSID_StartMenuCacheAndAppResolver && result != S_OK)
	{
		if (riid == IID_IAppResolver7)
		{
			//dbgprintf(L"Explorer_CoCreateInstance: Resolver7 using iappresolver8\n");
			PVOID rslvr8 = NULL;
			CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IAppResolver8, &rslvr8);
			//create our object

			CStartMenuResolver* resolver7 = new CStartMenuResolver((IAppResolver8*)rslvr8);
			result = resolver7->QueryInterface(riid, ppv);
			//if (result == S_OK)
				//dbgprintf(L"Explorer_CoCreateInstance: Resolver7 using iappresolver8 IS OK!!\n");
		}
		else if (riid == IID_IStartMenuItemsCache7)
		{
			int build = g_osVersion.BuildNumber();
			IID iid = IID_IStartMenuItemsCache8;
			if (build >= 14393)
				iid = IID_IStartMenuItemsCache10;

			void* newcache = nullptr;
			CoCreateInstance(rclsid, pUnkOuter, dwClsContext, iid, &newcache);

			CStartMenuResolver* resolver7 = nullptr;
			if (build >= 14393)
				resolver7 = new CStartMenuResolver((IStartMenuItemsCache10*)newcache);
			else
				resolver7 = new CStartMenuResolver((IStartMenuItemsCache8*)newcache);

			result = resolver7->QueryInterface(riid, ppv);
			if (result == S_OK)
				dbgprintf(L"Explorer_CoCreateInstance: Cache7 using IStartMenuItemsCache8/10 is OK!!\n");
		}
	}
	if ((rclsid == CLSID_StartMenuPin || rclsid == CLSID_TaskbarPin) /* && riid == IID_IPinnedList2*/ && result != S_OK)
	{
		int build = g_osVersion.BuildNumber();
		IID id = IID_IPinnedList25;

		if (build >= 14393 && build < 17763)
		{
			id = IID_IFlexibleTaskbarPinnedList;
		}
		else if (build >= 17763)
		{
			id = IID_IPinnedList3;
		}

		if (rclsid == CLSID_TaskbarPin && CTaskbandPin_CreateInstance && build >= 26100) // Windows 11...
		{
			CTaskbandPin_W32PTP* pTaskbandPin;
			result = CTaskbandPin_CreateInstance(&pTaskbandPin);
			dbgprintf(L"CTaskbandPin_CreateInstance result: %p", result);
			if (SUCCEEDED(result))
			{
				result = ((IUnknown*)pTaskbandPin)->QueryInterface(id, ppv);
				dbgprintf(L"CTaskbandPin_CreateInstance result 2: %p", result);
				((IUnknown*)pTaskbandPin)->Release();
			}
		}
		else
		{
			result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, id, ppv);
		}

		if (SUCCEEDED(result))
		{
			*ppv = new CPinnedListWrapper((IUnknown*)*ppv, build);
		}

	}

	if (riid == IID_AutoDestList && result != S_OK)
	{
		dbgprintf(L"USE 10 AUTODESTLIST!!!!\n");
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_AutoDestList10, ppv);
		*ppv = new CAutoDestWrapper((IAutoDestinationList10*)*ppv);
	}
	if (riid == IID_CustomDestList && result != S_OK)
	{
		dbgprintf(L"CUSTOMDESTLIST!!!!\n");
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_CustomDestList10, ppv);
		if (result != S_OK || !*ppv)
		{
			result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_CustomDestList1507, ppv);
			*ppv = new CCustomDestWrapper((IInternalCustomDestList1507*)*ppv);
		}
		else
			*ppv = new CCustomDestWrapper((IInternalCustomDestList10*)*ppv);
	}
	if (riid == IID_IShellTaskScheduler7)
	{
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
		dbgprintf(L"wrap IID_IShellTaskScheduler7\n");
		*ppv = new CShellTaskSchedulerWrapper((IShellTaskScheduler7*)*ppv);
	}
	if (result == S_OK && rclsid == CLSID_SysTray) //wrap stobject
	{
		dbgprintf(L"wrap stobject\n");
		*ppv = new CSysTrayWrapper((IOleCommandTarget*)*ppv);
	}
	if (rclsid == CLSID_AuthUIShutdownChoices && result != S_OK) //wrap authui
	{
		dbgprintf(L"wrap authui\n");
		int build = g_osVersion.BuildNumber();
		if (*ppv)
		{
			dbgprintf(L"good\n");
			*ppv = new CAuthUIWrapper((IUnknown*)*ppv, build);
		}
		else
		{
			IID dk = IID_IShutdownChoices8;
			if (build >= 10074)
				dk = IID_IShutdownChoices10;

			result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, dk, ppv);
			if (*ppv)
			{
				dbgprintf(L"good 2\n");
				*ppv = new CAuthUIWrapper((IUnknown*)*ppv, build);
			}
		}
	}
	if (riid == IID_TrayClock7 && result != S_OK)
		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_TrayClock8, ppv);

	return result;
}

extern "C" HRESULT WINAPI Explorer_CoRegisterClassObject(
	REFCLSID rclsid,     //Class identifier (CLSID) to be registered
	IUnknown* pUnk,     //Pointer to the class object
	DWORD dwClsContext,  //Context for running executable code
	DWORD flags,         //How to connect to the class object
	LPDWORD  lpdwRegister
)
{
	if (rclsid == CLSID_TrayNotify)
	{
		pUnk = new CTrayNotifyFactory((IClassFactory*)pUnk);
		if (g_osVersion.BuildNumber() < 10074 || s_EnableImmersiveShellStack == 2) // Ittr: gate fakeimmersive to 8.1 due to functional issues (e.g. hanging) with 10 - restoring this on 10 is now seemingly unnecessary
		{
			//register immersive shell fake too
			RegisterFakeImmersive();
			//and projection
			RegisterProjection();
		}
	}

	HRESULT rslt = CoRegisterClassObject(rclsid, pUnk, dwClsContext, flags, lpdwRegister);

	if (rclsid == CLSID_TrayNotify)
		dwRegisterNotify = *lpdwRegister;

	return rslt;
}

extern "C" HRESULT WINAPI Explorer_CoRevokeClassObject(DWORD dwRegister)
{
	if (dwRegister == dwRegisterNotify)
	{
		if (g_osVersion.BuildNumber() < 10074 || s_EnableImmersiveShellStack == 2) // Ittr: gate fakeimmersive to 8.1 due to functional issues (e.g. hanging) with 10
		{
			UnregisterFakeImmersive();
			UnregisterProjection();
		}
	}
	return CoRevokeClassObject(dwRegister);
}
