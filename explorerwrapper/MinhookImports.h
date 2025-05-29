#pragma once
#include "util.h"
#include "common.h"
#include "dbgprint.h"
#include "OptionConfig.h"
#include "OSVersion.h"
#include "TypeDefinitions.h"
#include "MinHook.h"
#include "NscTree.h"

HTHEME __stdcall OpenThemeData_Hook(HWND hwnd, LPCWSTR pszClassList)
{
	if (g_dwTrayThreadId > 0 && g_dwTrayThreadId != GetCurrentThreadId())
		return fOpenThemeData(hwnd, pszClassList);

	if (!AllowThemes())
		return NULL;

	LoadCurrentTheme(hwnd, pszClassList);

	if (g_currentTheme == nullptr)
		dbgprintf(L"OPENTHEMEDATA FAILED %s", pszClassList);

	themeHandles->push_back(g_currentTheme);
	return g_currentTheme;
}

HTHEME __stdcall OpenThemeDataForDpi_Hook(HWND hwnd, LPCWSTR pszClassList, UINT dpi)
{
	if (g_dwTrayThreadId > 0 && g_dwTrayThreadId != GetCurrentThreadId())
		return fOpenThemeDataForDpi(hwnd, pszClassList, dpi);

	if (!AllowThemes())
		return NULL;

	HTHEME theme = 0;
	DWORD flags = 2;
	if (dpi != 96)
		flags |= 1u;

	// Ittr: Windows 11 introduces issues with applying themes to the SearchFolder interface, due to the ItemsViewAccessible::Header addition
	// This is resolved by simply falling back to the system theme if the DirectUI theme call attempts to load this class
	if (g_loadedTheme && (lstrcmp(pszClassList, L"ItemsViewAccessible::Header") != 0))
	{
		theme = OpenThemeDataFromFile(g_loadedTheme, hwnd, pszClassList, flags);
	}
	else
	{
		theme = fOpenThemeDataForDpi(hwnd, pszClassList, dpi);
	}

	if (theme == nullptr)
		dbgprintf(L"OPENTHEMEDATAFORDPI FAILED %s", pszClassList);
	themeHandles->push_back(theme);
	return theme;
}

HTHEME __stdcall OpenThemeDataEx_Hook(HWND hwnd, LPCWSTR pszClassList, DWORD dwFlags)
{
	if (g_dwTrayThreadId > 0 && g_dwTrayThreadId != GetCurrentThreadId())
		return fOpenThemeDataEx(hwnd, pszClassList, dwFlags);

	if (!AllowThemes())
		return NULL;

	HTHEME theme = 0;
	DWORD flags = 2;
	if ((unsigned int)GetScreenDpi() != 96)
		flags |= 1u;

	if (g_loadedTheme)
		theme = OpenThemeDataFromFile(g_loadedTheme, hwnd, pszClassList, dwFlags | flags);
	else
		theme = fOpenThemeDataEx(hwnd, pszClassList, dwFlags);

	if (theme == nullptr)
		dbgprintf(L"OPENTHEMEDATAEX FAILED %s", pszClassList);
	themeHandles->push_back(theme);
	return theme;
}

void CPniMainDlg_ShowFlyoutNEW() // don't bother with the parameters as we aren't going to use them
{
	// Open Network and Sharing Center instead inside the Windows Control Panel, as a non-immersive alternative
	ShellExecuteW(nullptr, nullptr, L"control.exe", L"/name Microsoft.NetworkAndSharingCenter", nullptr, SW_SHOWNORMAL);

	// End function as we aren't going to do anything else here
	return;
}

void RenderThumbnail(PVOID This, int animoffset, int bNoRedraw)
{
	RECT rc = *(RECT*)((PBYTE)This + 0x68);
	HWND hwnd = *(HWND*)((PBYTE)This + 0x60);
	HTHEME hthem = *(HTHEME*)((PBYTE)This + 0x98);

	renderThumbnail_orig(This, animoffset, bNoRedraw);

	MARGINS mar;
	GetThemeMargins(hthem, NULL, 2, 0, TMT_CONTENTMARGINS, NULL, &mar);
	rc.left += mar.cxLeftWidth;
	rc.right -= mar.cxRightWidth;
	rc.top += mar.cyTopHeight;
	rc.bottom -= mar.cyBottomHeight;
	DwmpUpdateAccentBlurRect(hwnd, &rc);
}

HICON GetUWPIcon(HWND a2)
{
	HICON icon = NULL;
	IShellItemImageFactory* psiif = nullptr;
	IPropertyStore* ips;
	SHGetPropertyStoreForWindow(a2, IID_PPV_ARGS(&ips));
	GUID myGuid = { 0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3} };
	PROPERTYKEY propertyKey = { myGuid, 5 };
	PROPVARIANT pv;
	ips->GetValue(propertyKey, &pv);
	if (pv.vt == VT_LPWSTR)
	{
		LPCWSTR aumid = pv.pwszVal;
		SHCreateItemInKnownFolder(FOLDERID_AppsFolder, KF_FLAG_DONT_VERIFY, aumid, IID_PPV_ARGS(&psiif));
		if (psiif)
		{
			SIIGBF flags = SIIGBF_ICONONLY;
			HBITMAP hb;
			SIZE size = { GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON) };
			HRESULT hr = psiif->GetImage(size, flags, &hb);
			if (SUCCEEDED(hr))
			{
				HIMAGELIST hImageList = ImageList_Create(size.cx, size.cy, ILC_COLOR32, 1, 0);
				if (ImageList_Add(hImageList, hb, NULL) != -1)
				{
					HICON hc = ImageList_GetIcon(hImageList, 0, 0);
					ImageList_Destroy(hImageList);

					// set
					icon = hc;

					DeleteObject(hb);
					psiif->Release();
				}
				DeleteObject(hb);
			}
			psiif->Release();
		}
	}
	ips->Release();
	return icon;
}

PVOID CTaskBandPtr = 0;

VOID SetWindowIcon(PVOID This, HWND a2, HICON a3, int a4)
{
	CTaskBandPtr = This;
	if (IsShellFrameWindow && IsShellFrameWindow(a2))
	{
		HICON hc = GetUWPIcon(a2);
		if (hc) SetIcon(This, a2, hc, a4);
	}
	else
		SetIcon(This, a2, a3, a4);
}

VOID UpdateItemIcon(PVOID This, int a2)
{
	typedef void* (__fastcall* GetTaskItemFunc)(void*);
	typedef HWND(__fastcall* GetWindowFunc)(void*);

	HDPA hdpaTaskThumbnails = *(HDPA*)((PBYTE)This + 0xB0);
	auto v4 = DPA_FastGetPtr(hdpaTaskThumbnails, a2);
	auto vtable = *(uintptr_t**)v4;
	GetTaskItemFunc GetTaskItem = (GetTaskItemFunc)vtable[0x60 / sizeof(uintptr_t)];
	void* v5 = GetTaskItem(v4);
	auto vtable_v5 = *(uintptr_t**)v5;
	GetWindowFunc GetWindow = (GetWindowFunc)vtable_v5[0x98 / sizeof(uintptr_t)];
	HWND v6 = GetWindow(v5);
	if (IsShellFrameWindow && IsShellFrameWindow(v6))
	{
		HICON hc = GetUWPIcon(v6);
		if (hc) SetIconThumb(This, hc, a2, 3);
	}
	else
		UpdateItem(This, a2);

}

// Ittr: Under immersive mode, the differences in ShellHook operation have to be accounted for
HRESULT(__fastcall* OnShellHookMessage)(void* a1);

bool fShowLauncher = false; // Ittr: First run erroneously shows the start menu, unless we handle it differently

HRESULT OnShellHookMessage_Hook(void* a1) // This gets called when start menu is to be opened - has been a bit temperamental
{
	// Use of fShowLauncher flag is essential to have something resembling stability for overall functionality
	if (fShowLauncher) // If the flag is set...
	{
		PostMessageW(hwnd_taskbar, 0x504, 0, 0); // Fire the message directly that opens Windows 7's start menu - ShellHook unreliable pre-VB
		return S_OK; // Ensure the run is recognised as a success 
	}
	else // However, without the flag, this is presumed to be the first run
	{
		fShowLauncher = true; // Enable the flag for showing the start menu now that this first attempt has run through
		return E_FAIL;  // Then, ensure this run is marked as a failure
	}

	return OnShellHookMessage(a1); // This codepath should ideally never run
}

void SetUpThemeManager()
{
	// Initialize the theme manager and declare the types for the UXTheme apis we're hooking
	ThemeManagerInitialize();

	fOpenThemeData = decltype(fOpenThemeData)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeData"));
	fOpenThemeDataForDpi = decltype(fOpenThemeDataForDpi)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeDataForDpi"));
	fOpenThemeDataEx = decltype(fOpenThemeDataEx)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeDataEx"));

	// Hook UXTheme-related calls for the purpose of our inactive theme system.
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeData), OpenThemeData_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeData));
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeDataForDpi), OpenThemeDataForDpi_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeDataForDpi));
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeDataEx), OpenThemeDataEx_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeDataEx));
}

void FixNonImmersivePniDui()
{
	if (g_osVersion.BuildNumber() >= 10074) // not needed for 8.1
	{
		// Unable to do with patterns alone, as Microsoft removed HrOpenControlPanel
		if (!s_UseDCompFlyouts || !s_EnableImmersiveShellStack)
		{
			HMODULE pnidui = LoadLibrary(L"pnidui.dll");

			if (pnidui) // only run if DLL is present - handled like this because GE removes pnidui...
			{
				void* _ShowFlyout = (void*)FindPattern((uintptr_t)LoadLibrary(L"pnidui.dll"), "48 89 6C 24 18 56 57 41 57 48 83 EC 60");

				if (_ShowFlyout) // first run, VB to NI
				{
					MH_CreateHook(static_cast<LPVOID>(_ShowFlyout), CPniMainDlg_ShowFlyoutNEW, reinterpret_cast<LPVOID*>(&CPniMainDlg_ShowFlyout));
				}
				else
				{
					_ShowFlyout = (void*)FindPattern((uintptr_t)LoadLibrary(L"pnidui.dll"), "48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 40 8A");

					if (_ShowFlyout) // second run, RS4 to TI
					{
						MH_CreateHook(static_cast<LPVOID>(_ShowFlyout), CPniMainDlg_ShowFlyoutNEW, reinterpret_cast<LPVOID*>(&CPniMainDlg_ShowFlyout));
					}
					else
					{
						_ShowFlyout = (void*)FindPattern((uintptr_t)LoadLibrary(L"pnidui.dll"), "48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 20 40 8A FA");

						if (_ShowFlyout) // third run, TH2 to RS3
						{
							MH_CreateHook(static_cast<LPVOID>(_ShowFlyout), CPniMainDlg_ShowFlyoutNEW, reinterpret_cast<LPVOID*>(&CPniMainDlg_ShowFlyout));
						}
						else
						{
							_ShowFlyout = (void*)FindPattern((uintptr_t)LoadLibrary(L"pnidui.dll"), "48 8B C4 56 57 41 56 48 81 EC 80 01 00 00");

							if (_ShowFlyout) // fourth run, TH1
							{
								MH_CreateHook(static_cast<LPVOID>(_ShowFlyout), CPniMainDlg_ShowFlyoutNEW, reinterpret_cast<LPVOID*>(&CPniMainDlg_ShowFlyout));
							}
						}
					}
				}
			}
		}
	}
}

void UpdateTrayWindowDefinitions()
{
	// Hook and update definitions of what windows should be added to the tray - largely for UWP purposes, but essentially zero-cost so included on both immersive on and off modes.
	void* _ShouldAddWindowToTray = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8B F9 33 DB");
	void* _IsWindowNotDesktopOrTray = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B F9 33 DB FF 15 ?? ?? ?? ?? 3B C3 74 ?? 48 3B 3D");
	MH_CreateHook(static_cast<LPVOID>(_ShouldAddWindowToTray), ShouldAddWindowToTray, reinterpret_cast<LPVOID*>(&_ShouldAddWindowToTray));
	MH_CreateHook(static_cast<LPVOID>(_IsWindowNotDesktopOrTray), IsWindowNotDesktopOrTray, reinterpret_cast<LPVOID*>(&_IsWindowNotDesktopOrTray));
}

void SetProgramListNscTreeAttributes()
{
	// If we are on Windows 10 or higher, query the original program list pattern and create our hook to fix the visual issues
	if (g_osVersion.BuildNumber() >= 10074)
	{
		CNSCHost_FillNSCOg = (decltype(CNSCHost_FillNSCOg))FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 18 57 48 83 EC 30 33 DB 48 8B F9 39 99 CC 00 00 00");
		if (CNSCHost_FillNSCOg)
			MH_CreateHook(static_cast<LPVOID>(CNSCHost_FillNSCOg), CNSCHost_FillNSC, reinterpret_cast<LPVOID*>(&CNSCHost_FillNSCOg)); //this hook is in nsctree.h now
	}
}

void HandleThumbnailColorization()
{
	// CTaskListThumbnailWnd::_Render
	// Thumbnail rendering fix for colorization modes
	if (g_osVersion.BuildNumber() >= 10074) // we don't apply to 8.1 as only pseudo-aero is supported there
	{
		char* CTaskListThumbnailWnd_Render = "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 20 44 89 40 18 57 41 54 41 55 41 56 41 57 48 81 EC 90 00 00 00 48 8B F9";
		void* CTLWRPattern = (void*)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskListThumbnailWnd_Render);

		if (CTLWRPattern)
		{
			MH_CreateHook(static_cast<LPVOID>(CTLWRPattern), RenderThumbnail, reinterpret_cast<LPVOID*>(&renderThumbnail_orig));
		}
		else // 7779 and 7785
		{
			CTaskListThumbnailWnd_Render = "48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC A0 00 00 00 48 8B 05 ?? ?? 04 00 48 33 C4";
			CTLWRPattern = (void*)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskListThumbnailWnd_Render);

			if (CTLWRPattern)
			{
				MH_CreateHook(static_cast<LPVOID>(CTLWRPattern), RenderThumbnail, reinterpret_cast<LPVOID*>(&renderThumbnail_orig));
			}
		}
	}
}

void RenderStoreAppsOnTaskbar()
{
	if (s_ShowStoreAppsOnTaskbar && g_osVersion.BuildNumber() >= 10074)
	{
		// Part 1: CTaskListThumbnailWnd::_SetIcon
		// Must be defined so that it can be called by our hook functions
		// However, we only assign the definition if we can actually detect it to begin with
		char* CTaskListThumbnailWnd_SetIcon = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 63 D8 4C 8B 81 B0 00 00 00";
		setIconThumb_t CTLTWSIPattern = (setIconThumb_t)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskListThumbnailWnd_SetIcon);

		if (CTLTWSIPattern)
		{
			SetIconThumb = CTLTWSIPattern;
		}
		else // 7779 and 7785
		{
			CTaskListThumbnailWnd_SetIcon = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B 81 B0 00 00 00 49 63 D8";
			CTLTWSIPattern = (setIconThumb_t)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskListThumbnailWnd_SetIcon);

			if (CTLTWSIPattern)
			{
				SetIconThumb = CTLTWSIPattern;
			}
			else
			{
				// In this case if we are unable to find the definition, return here so the function doesn't apply the hooks
				// This prevents stability issues if we are unable to define this properly
				return;
			}
		}

		// Part 2: CTaskBand::_SetWindowIcon 
		// Must be hooked accordingly so the icon can be overridden as necessary for the TaskItem buttons
		char* CTaskBand_SetWindowIcon = "FF F3 55 56 57 41 54 41 55 41 56 41 57 48 81 EC F8 06 00 00";
		void* CTBSWIPattern = (void*)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskBand_SetWindowIcon);

		if (CTBSWIPattern)
		{
			MH_CreateHook(static_cast<LPVOID>(CTBSWIPattern), SetWindowIcon, reinterpret_cast<LPVOID*>(&SetIcon));
		}
		else // 7779 and 7785
		{
			CTaskBand_SetWindowIcon = "4C 8B DC 49 89 5B 08 49 89 73 10 49 89 7B 18 4D 89 63 20 55 48 8B EC";
			CTBSWIPattern = (void*)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskBand_SetWindowIcon);

			if (CTBSWIPattern)
			{
				MH_CreateHook(static_cast<LPVOID>(CTBSWIPattern), SetWindowIcon, reinterpret_cast<LPVOID*>(&SetIcon));
			}
		}

		// Part 3: CTaskListThumbnailWnd::_UpdateItemIcon
		// Hooking this function will allow the thumbnail icon to be updated as applicable
		char* CTaskListThumbnailWnd_UpdateItemIcon = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 48 8B 81 B0 00 00 00";
		void* CTLTWUIIPattern = (void*)FindPattern((uintptr_t)GetModuleHandle(NULL), CTaskListThumbnailWnd_UpdateItemIcon);

		if (CTLTWUIIPattern)
		{
			MH_CreateHook(static_cast<LPVOID>(CTLTWUIIPattern), UpdateItemIcon, reinterpret_cast<LPVOID*>(&UpdateItem));
		}
	}
}

void CreateImmersiveShell()
{
	// NOTE: Some of the patch functions are in util.h rather than an imports header because they are used with several patch types
	////////////////////////////////
	// 1. Todo in future *after* feature-set is complete: see how many of these hooks can be ChangeImportedAddress instead of MH_CreateHook (perf optimisation)
	// 2. Code stack used exclusively for UWP mode, hence the conditional statement.
	if (s_EnableImmersiveShellStack == 1) // Run these hooks only if the user has UWP enabled
	{
		// 1. This will *need* serious optimization in the near future as it singlehandedly delays program enumeration and startup by several seconds
		// 2. Prepare the taskbar and thumbnails to handle UWP icons. Further work needed for jumplists and to prevent wrongful classification as "Application Frame Host" in the first place.

		// The rest of this code block is dedicated to ensuring UWP actually runs in the first place
		CreateWindowInBandOrig = decltype(CreateWindowInBandOrig)(GetProcAddress(GetModuleHandle(L"user32.dll"), "CreateWindowInBand"));
		CreateWindowInBandExOrig = decltype(CreateWindowInBandExOrig)(GetProcAddress(GetModuleHandle(L"user32.dll"), "CreateWindowInBandEx"));
		SetWindowBandApiOrg = decltype(SetWindowBandApiOrg)(GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowBand"));
		RegisterHotKeyApiOrg = decltype(RegisterHotKeyApiOrg)(GetProcAddress(GetModuleHandle(L"user32.dll"), "RegisterHotKey"));

		MH_CreateHook(static_cast<LPVOID>(CreateWindowInBandOrig), CreateWindowInBandNew, reinterpret_cast<LPVOID*>(&CreateWindowInBandOrig));
		MH_CreateHook(static_cast<LPVOID>(CreateWindowInBandExOrig), CreateWindowInBandExNew, reinterpret_cast<LPVOID*>(&CreateWindowInBandExOrig));
		MH_CreateHook(static_cast<LPVOID>(SetWindowBandApiOrg), SetWindowBandNew, reinterpret_cast<LPVOID*>(&SetWindowBandApiOrg));
		MH_CreateHook(static_cast<LPVOID>(RegisterHotKeyApiOrg), RegisterWindowHotkeyNew, reinterpret_cast<LPVOID*>(&RegisterHotKeyApiOrg));

		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2581), RetTrue, NULL); // GetWindowTrackInfoAsync
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2563), RetTrue, NULL); // ClearForeground
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2628), RetTrue, NULL); // CreateWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2629), RetTrue, NULL); // DeleteWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2631), RetTrue, NULL); // EnableWindowGroupPolicy
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2627), RetTrue, NULL); // SetBridgeWindowChild
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2511), RetTrue, NULL); // SetFallbackForeground
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2566), RetTrue, NULL); // SetWindowArrangement
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2632), RetTrue, NULL); // SetWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2579), RetTrue, NULL); // SetWindowShowState
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2585), RetTrue, NULL); // UpdateWindowTrackingInfo
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2514), RetTrue, NULL); // RegisterEdgy
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2542), RetTrue, NULL); // RegisterShellPTPListener
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2537), RetTrue, NULL); // SendEventMessage
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2513), RetTrue, NULL); // SetActiveProcessForMonitor
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2564), RetTrue, NULL); // RegisterWindowArrangementCallout
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2567), RetTrue, NULL); // EnableShellWindowManagementBehavior
	}
}

void _OnHShellTaskMan()
{
	if (s_EnableImmersiveShellStack == 1)
	{
		// Here we account for the immersive shell's destructive impacts upon certain internal mechanisms of explorer

		// Work out what we need for different OS versions (TH1 through GE)
		char* XamlLauncher_OnShellHookMessage;
		char* XLOSHMPattern;

		// Check whether the modern DLL exists in Windows
		HMODULE twinUI_PCShell = LoadLibrary(L"twinui.pcshell.dll");
		if (twinUI_PCShell) // If it does...
		{
			XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 83 C1 D8 33 D2 E8 ?? ?? ?? ?? 8B D8";
			XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

			if (XLOSHMPattern) // NI, GE
			{
				MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
			}
			else
			{
				XamlLauncher_OnShellHookMessage = "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 76 48 8B 01";
				XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

				if (XLOSHMPattern) // CO
				{
					MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
				}
				else
				{
					XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 48 8B 40 ?? FF 15 ?? ?? ?? ?? 84 C0 0F 85 ?? ?? ?? ?? 38 83";
					XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

					if (XLOSHMPattern) // VB
					{
						MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
					}
					else
					{
						XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 4E 48 8B 01";
						XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

						if (XLOSHMPattern) // RS5, 19H1
						{
							MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
						}
						else
						{
							XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 59 48 8B 01"; // 0x59 cannot be wildcarded
							XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

							if (XLOSHMPattern) // RS4
							{
								MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
							}
							else
							{
								XamlLauncher_OnShellHookMessage = "48 89 5C 24 10 57 48 83 EC 30 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 75 0A";
								XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

								if (XLOSHMPattern) // RS3
								{
									MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
								}
								else
								{
									XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 75 07 B8 90 04 07 80 EB 6F 48 8B 01";
									XLOSHMPattern = (char*)FindPattern((uintptr_t)twinUI_PCShell, XamlLauncher_OnShellHookMessage);

									if (XLOSHMPattern) // RS2
									{
										MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
									}
									else
									{
										goto _OnHShellTaskMan_TWINUI; // New DLL exists on RS1 but unused for these purposes. Fall back to twinui.dll
									}
								}
							}
						}
					}
				}
			}
		}
		else // RS1 and earlier
		{
_OnHShellTaskMan_TWINUI:
			HMODULE twinui = LoadLibrary(L"twinui.dll");

			if (twinui) // This should always exist, but we check in case it doesn't
			{
				XamlLauncher_OnShellHookMessage = "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 ?? ?? ?? ?? ?? ?? 48 8B 01";
				XLOSHMPattern = (char*)FindPattern((uintptr_t)twinui, XamlLauncher_OnShellHookMessage);

				if (XLOSHMPattern) // RS1
				{
					MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
				}
				else
				{
					XamlLauncher_OnShellHookMessage = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B B9 ?? ?? ?? ?? 48 8B D9 48 85 FF ?? ?? ?? ?? ?? ?? 48 8B 07";
					XLOSHMPattern = (char*)FindPattern((uintptr_t)twinui, XamlLauncher_OnShellHookMessage);

					if (XLOSHMPattern) // TH2
					{
						MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));
					}
					else
					{
						XamlLauncher_OnShellHookMessage = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B B9 ?? ?? ?? ?? 48 8B D9 48 85 FF 74 62";
						char* CImmersiveLauncher_OnShellHookMessage = "48 89 5C 24 10 55 56 57 48 8B EC 48 83 EC 20 FF"; // Server uses this

						XLOSHMPattern = (char*)FindPattern((uintptr_t)twinui, XamlLauncher_OnShellHookMessage);
						char* CILOSHMPattern = (char*)FindPattern((uintptr_t)twinui, CImmersiveLauncher_OnShellHookMessage);

						if (XLOSHMPattern) // TH1
						{
							MH_CreateHook(static_cast<LPVOID>(XLOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&XLOSHMPattern));

							if (CILOSHMPattern) // Second stage for Server SKUs that use legacy CImmersiveLauncher
							{
								MH_CreateHook(static_cast<LPVOID>(CILOSHMPattern), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&CILOSHMPattern));
							}
						}
					}
				}
			}
		}
	}
}

void ChangeMinhookImports()
{
	MH_Initialize();

	SetUpThemeManager(); // Local visual style management init
	FixNonImmersivePniDui(); // Non-immersive network flyout handling
	UpdateTrayWindowDefinitions(); // Ensure tray exclusion is corrected for modern Windows
	SetProgramListNscTreeAttributes(); // Restore the relevant contents to the program list
	HandleThumbnailColorization(); // Thumbnail colorization to match
	RenderStoreAppsOnTaskbar(); // UWP icon rendering for the taskbar
	CreateImmersiveShell(); // Immersive shell initialisation
	_OnHShellTaskMan(); // Handling the immersive shell's impacts on the holographic shell and associated ShellHook messages
}
