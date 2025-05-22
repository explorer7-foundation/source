#include "PatternImports.h"

// Remove AMAP class from loaded msstyle so that Vista and 7 msstyles are compatible
void RemoveLoadAnimationDataMap()
{
	// thank you amrsatrio for the pattern + offsetting method
	char* LoadAnimationDataMap = "48 8B 53 20 48 8B ?? E8 ?? ?? ?? ?? 8B ?? 48 8B";

	HMODULE uxTheme = GetModuleHandle(L"uxtheme.dll");
	if (uxTheme)
	{
		char* LADMPattern = (char*)FindPattern((uintptr_t)uxTheme, LoadAnimationDataMap);

		if (LADMPattern)
		{
			LADMPattern += 7;
			LADMPattern += 5 + *(int*)(LADMPattern + 1);

			unsigned char bytes[] = { 0x31, 0xC0, 0xC3 };
			ChangeImportedPattern(LADMPattern, bytes, sizeof(bytes));
		}
	}
}

// For Windows 8.1 - remove additional Immersive class from loaded msstyle so that Vista and 7 msstyles are compatible
void RemoveGetClassIdForShellTarget()
{
	char* GetClassIdForShellTarget = "4C 8B DC 4D 89 43 18 49 89 4B 08 53 48 83 EC 30";

	HMODULE uxTheme = GetModuleHandle(L"uxtheme.dll");
	if (uxTheme)
	{
		char* GCIFSTPattern = (char*)FindPattern((uintptr_t)uxTheme, GetClassIdForShellTarget);

		if (GCIFSTPattern)
		{
			unsigned char bytes[] = { 0x31, 0xC0, 0xC3 };
			ChangeImportedPattern(GCIFSTPattern, bytes, sizeof(bytes));
		}
	}
}

// Fix CLogoffPane so that options are correctly displayed
void FixAuthUI()
{
	// Newer explorer versions use this
	// CLogoffPane::_InitShutdownObjects
	const char* bytes = "48 8B ?? 98 00 00 00 48 8B ?? 40 45 33 C0 48 8B 01 FF 50 18 "
		"48 8B ?? 98 00 00 00 48 8B 01 FF 50 30 8B D8 "
		"85 C0 ?? ?? "
		"48 8B ?? 98 00 00 00 48 8D ?? A0 00 00 00 48 8B 01 FF 50 20";

	// Older explorer versions use this
	// CLogoffPane::_OnCreate
	const char* bytesOld = "48 8B 8B 98 00 00 00 48 8B 53 40 45 33 C0 48 8B 01 FF 50 18 "
		"48 8B 8B 98 00 00 00 48 8B 01 FF 50 30 44 8B C8 "
		"85 C0 ?? ?? ?? ?? ?? ?? "
		"48 8B 8B 98 00 00 00 48 8D 93 A0 00 00 00 48 8B 01 FF 50 20";

	char* pattern = (char*)FindPattern((uintptr_t)GetModuleHandle(NULL), bytes);
	char* pattern1 = (char*)FindPattern((uintptr_t)GetModuleHandle(NULL), bytesOld);

	unsigned char patch1[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	SIZE_T size = sizeof(patch1);
	if (pattern)
	{
		// mov rax, [rcx]
		// call qword ptr [rax+18h]
		char* inst1 = pattern + 14;
		ChangeImportedPattern(inst1, patch1, size);

		// mov rax, [rcx]
		// call qword ptr [rax+30h]
		char* inst2 = pattern + 27;
		ChangeImportedPattern(inst2, patch1, size);

		// mov rax, [rcx]
		// call qword ptr [rax+20h]
		char* inst3 = pattern + 53;
		ChangeImportedPattern(inst3, patch1, size);
	}

	if (pattern1 && !pattern) // Ittr: Only apply to CLogoffPane::_OnCreate if we need to, otherwise this causes crashing on later 7 explorer.
	{
		// mov rax, [rcx]
		// call qword ptr [rax+18h]
		char* inst1 = pattern1 + 14;
		ChangeImportedPattern(inst1, patch1, size);

		// mov rax, [rcx]
		// call qword ptr [rax+30h]
		char* inst2 = pattern1 + 27;
		ChangeImportedPattern(inst2, patch1, size);

		// mov rax, [rcx]
		// call qword ptr [rax+20h]
		char* inst3 = pattern1 + 58;
		ChangeImportedPattern(inst3, patch1, size);
	}
}

// Ittr: Get rid of the immersive start menu and stop it appearing on TH1+ when immersive shell is on.
// This is very important for good user experience.
// I'll also be honest - I haven't tested 1703 because who actually uses 1703
void DisableImmersiveStart()
{
	if (s_EnableImmersiveShellStack == 1) // don't run this if the user isn't using immersive shell
	{
		char* ShowStartView; // XamlLauncher::ShowStartView
		char* SSVPattern;
		unsigned char bytes[] = { 0xC3 }; // retn

		// Load twinui.pcshell.dll for versions where it is used...
		// Otherwise we load twinui.dll (see further below)
		HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

		if (twinui_pcshell) // only run if DLL is present
		{
			ShowStartView = "40 55 53 56 57 41 56 48 8D 6C 24 80 48 81 EC 80 01 00 00 48 8B 05 06 39 8E 00 48 33 C4";
			SSVPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, ShowStartView);

			if (SSVPattern) // first run, late NI
			{
				ChangeImportedPattern(SSVPattern, bytes, sizeof(bytes));
			}
			else
			{
				ShowStartView = "48 89 5C 24 20 55 56 57 48 81 EC ?? 01 00 00 48 8B 05 ?? ?? ?? 00 48 33 C4 48 89 84 24 ?? 01 00 00 48 83 B9 ?? ?? 00 00 00";
				SSVPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, ShowStartView);

				if (SSVPattern) // second run, RS3 to early NI
				{
					ChangeImportedPattern(SSVPattern, bytes, sizeof(bytes));
				}
				else
				{
					ShowStartView = "48 89 5C 24 20 55 56 57 48 83 EC 30 48 83 B9 F8 00 00 00 00 41 8B E8";
					SSVPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, ShowStartView);

					if (SSVPattern) // third run, RS2
					{
						ChangeImportedPattern(SSVPattern, bytes, sizeof(bytes));
					}
					else
					{
						// RS1 where twinui.pcshell.dll exists, but isn't used for this so we have to go to twinui version
						// this is an attempt to avoid additional build checks where they aren't needed
						goto DisableImmersiveStart_TWINUI;
					}
				}
			}
		}
		else // fourth run, TH1 to RS1, before twinui.pcshell was used for this...
		{
DisableImmersiveStart_TWINUI:
			HMODULE twinui = LoadLibrary(L"twinui.dll");

			if (twinui)
			{
				ShowStartView = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 83 B9 ?? 00 00 00 00 41 8B E8";
				SSVPattern = (char*)FindPattern((uintptr_t)twinui, ShowStartView);

				if (SSVPattern)
				{
					ChangeImportedPattern(SSVPattern, bytes, sizeof(bytes));
				}
			}
		}
	}

}

// Ittr: Remove the immersive search interface and prevent it appearing on TH1+ with ImmersiveShell enabled
// Otherwise, when invoked, it can take up half of the screen
// The Windows 7 start menu search functionality is much superior to this in any case
void DisableImmersiveSearch()
{
	if (s_EnableImmersiveShellStack == 1)
	{
		char* CortanaDesktopExperienceView_ShowInternal;
		char* CDEVSIPattern;
		char* XamlLauncherState_ShowSearchFromOpenStart;
		char* XLSSSFOSPattern;
		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };

		HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

		// Disable CortanaDesktopExperienceView and ShowSearchFromOpenStart
		if (twinui_pcshell)
		{
			// Disable ShowCortanaFromOpenStart from 19H1 onwards (later renamed ShowSearchFromOpenStart)
			XamlLauncherState_ShowSearchFromOpenStart = "48 89 54 24 10 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC";
			XLSSSFOSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, XamlLauncherState_ShowSearchFromOpenStart);

			if (XLSSSFOSPattern) // 22H2 and later
			{
				ChangeImportedPattern(XLSSSFOSPattern, bytes, sizeof(bytes));
			}
			else
			{
				XamlLauncherState_ShowSearchFromOpenStart = "48 89 54 24 10 55 53 56 57 41 56 41 57 48 8B EC 48 83 EC";
				XLSSSFOSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, XamlLauncherState_ShowSearchFromOpenStart);

				if (XLSSSFOSPattern) // VB to 21H2
				{
					ChangeImportedPattern(XLSSSFOSPattern, bytes, sizeof(bytes));
				}
				else
				{
					XamlLauncherState_ShowSearchFromOpenStart = "48 89 54 24 10 55 53 56 57 41 56 48 8B EC 48 83 EC 40 48 C7 45 E0 FE FF FF FF";
					XLSSSFOSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, XamlLauncherState_ShowSearchFromOpenStart);

					if (XLSSSFOSPattern) // 19H1 to 19H2
					{
						ChangeImportedPattern(XLSSSFOSPattern, bytes, sizeof(bytes));
					}
				}
			}

			// Now proceed to disabling CortanaDesktopExperienceView (used in TH1 to RS5, still present but unused)
			CortanaDesktopExperienceView_ShowInternal = "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 20 57 48 83 EC 20 41 8B ?? 41 8B ?? 48 8B FA";
			CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, CortanaDesktopExperienceView_ShowInternal);

			if (CDEVSIPattern) // VB and later
			{
				ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
			}
			else
			{
				CortanaDesktopExperienceView_ShowInternal = "40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 81 EC 80 00 00 00";
				CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, CortanaDesktopExperienceView_ShowInternal);

				if (CDEVSIPattern) // 19H1 to 19H2
				{
					ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
				}
				else
				{
					CortanaDesktopExperienceView_ShowInternal = "40 55 53 56 57 41 56 48 8D 6C 24 C9 48 81 EC 90 00 00 00";
					CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, CortanaDesktopExperienceView_ShowInternal);

					if (CDEVSIPattern) // RS4 to RS5
					{
						ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
					}
					else
					{
						CortanaDesktopExperienceView_ShowInternal = "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 20 41 8B D9 41 8B E8 48 8B F2";
						CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, CortanaDesktopExperienceView_ShowInternal);

						if (CDEVSIPattern) // RS3
						{
							ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
						}
						else
						{
							// RS1 where twinui.pcshell.dll exists, but isn't used for this so we have to go to twinui version
							// this is an attempt to avoid additional build checks where they aren't needed
							goto DisableImmersiveSearch_TWINUI;
						}
					}
				}
			}
		}
		else // TH1 to RS1
		{
DisableImmersiveSearch_TWINUI:
			HMODULE twinui = LoadLibrary(L"twinui.dll");

			if (twinui)
			{
				CortanaDesktopExperienceView_ShowInternal = "40 55 53 56 57 41 56 48 8B EC 48 83 EC 70";
				CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui, CortanaDesktopExperienceView_ShowInternal);

				if (CDEVSIPattern) // RS1
				{
					ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
				}
				else
				{
					CortanaDesktopExperienceView_ShowInternal = "48 8B C4 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? 48 81 EC 90 00 00 00 48 C7 45 DF FE FF FF FF 48 89 58 20";
					CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui, CortanaDesktopExperienceView_ShowInternal);

					if (CDEVSIPattern) // TH2
					{
						ChangeImportedPattern(CDEVSIPattern, bytes, sizeof(bytes));
					}
					else // no way of making a cross-compatible pattern despite a one-byte difference, thanks Microsoft
					{
						CortanaDesktopExperienceView_ShowInternal = "48 8B C4 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? 48 81 EC 90 00 00 00 48 C7 45 E7 FE FF FF FF";
						CDEVSIPattern = (char*)FindPattern((uintptr_t)twinui, CortanaDesktopExperienceView_ShowInternal);

						if (CDEVSIPattern) // TH1
						{
							unsigned char ThresholdBytes[]  = { 0xB0, 0x00, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
							ChangeImportedPattern(CDEVSIPattern, ThresholdBytes, sizeof(ThresholdBytes));
						}
					}
				}
			}
		}
	}
}

// Ittr: Get rid of the half-broken TaskView interface and prevent it appearing on TH1+ when UWP is on.
// This isn't detrimental to user experience to enable, but it completely breaks with the intended user experience.
// TaskView also causes crashing on earlier (pre-GE) Windows 11 which we can now avoid by disabling the remains of the feature.
// For some reason, Germanium and later already disable this. We're not complaining.
void DisableTaskView()
{
	if (s_EnableImmersiveShellStack == 1) // this on its own should be enough as we enforce this value to 0 prior to TH1
	{
		// For historical context, the relevant function has changed name a few times...
		// We opt to use the latest name throughout however the others are listed below:
		// 22H2 and later: TaskViewHost::Show
		// RS2 to 21H2: XamlAllUpViewHost::Show
		// TH1 to RS1: CAllUpViewHost::Show
		char* TaskViewHostShow;
		char* TVHSPattern;
		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 }; // mov al 0, retn - running retn on its own here has inconsistent results

		HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

		if (twinui_pcshell)
		{
			TaskViewHostShow = "40 53 56 57 41 54 41 55 41 56 41 57 48 81 EC 30 03 00 00";
			TVHSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, TaskViewHostShow);

			if (TVHSPattern) // 22H2 and later
			{
				ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
			}
			else
			{
				TaskViewHostShow = "48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 81 EC 20"; // not working in this run, needs further work
				TVHSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, TaskViewHostShow);

				if (TVHSPattern) // 21H2 (Windows 11)
				{
					ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
				}
				else
				{
					TaskViewHostShow = "48 89 5C 24 20 56 57 41 54 41 55 41 57 48 81 EC";
					TVHSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, TaskViewHostShow);

					if (TVHSPattern) // VB
					{
						ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
					}
					else
					{
						TaskViewHostShow = "4C 8B DC 57 41 54 41 55 41 56 41 57 48 81 EC 40 03 00 00";
						TVHSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, TaskViewHostShow);

						if (TVHSPattern) // RS5 to 19H2
						{
							ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
						}
						else
						{
							TaskViewHostShow = "4C 8B DC ?? 41 54 41 55 41 56 41 57 48 83 EC";
							TVHSPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, TaskViewHostShow);

							if (TVHSPattern) // RS2 to RS4
							{
								ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
							}
							else
							{
								// RS1 where twinui.pcshell.dll exists, but isn't used for this so we have to go to twinui version
								// this is an attempt to avoid additional build checks where they aren't needed
								goto DisableTaskView_TWINUI;
							}
						}
					}
				}
			}
		}
		else // TH1 to RS1, where twinui is used instead
		{
DisableTaskView_TWINUI:
			HMODULE twinui = LoadLibrary(L"twinui.dll");

			if (twinui)
			{
				TaskViewHostShow = "48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? ?? ?? ?? ?? 48 81 EC 50 02 00 00";
				TVHSPattern = (char*)FindPattern((uintptr_t)twinui, TaskViewHostShow);

				if (TVHSPattern) // TH2 to RS1
				{
					ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
				}
				else
				{
					TaskViewHostShow = "48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? ?? ?? ?? ?? 48 81 EC E0 01 00 00";
					TVHSPattern = (char*)FindPattern((uintptr_t)twinui, TaskViewHostShow);

					if (TVHSPattern) // TH1
					{
						ChangeImportedPattern(TVHSPattern, bytes, sizeof(bytes));
					}
				}
			}
		}
	}
}

// Ittr: Remove broken leftovers of immersive context menus, starting in Windows 10.
// Also to be noted that Windows 11 makes further changes here that we have to account for.
void RestoreWin32Menus()
{
	// Refactor: Do this in two separate parts - more readable but more lines of code (compiler should optimise...)
	// Pattern variables initialised locally in both to prevent race-conditions
	char unsigned bytes[] = { 0xB0, 0x00, 0xC3 }; // mov al 0, retn - running retn on its own here has inconsistent results
	
	// SHELL32
	HMODULE shell32 = GetModuleHandle(L"shell32.dll");

	if (shell32)
	{
		char* CanApplyOwnerDrawToMenu = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 33 DB 48 8B F2 33 FF 48 8B E9";
		char* CAODTMPattern = (char*)FindPattern((uintptr_t)shell32, CanApplyOwnerDrawToMenu);

		if (CAODTMPattern) // 24H2 and later
		{
			ChangeImportedPattern(CAODTMPattern, bytes, sizeof(bytes));
		}
		else
		{
			char* CanApplyOwnerDrawToMenu = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B E9 33 FF 33 D2";
			char* CAODTMPattern = (char*)FindPattern((uintptr_t)shell32, CanApplyOwnerDrawToMenu);

			if (CAODTMPattern) // TH1 to 23H2
			{
				ChangeImportedPattern(CAODTMPattern, bytes, sizeof(bytes));
			}
		}
	}

	// EXPLORERFRAME
	HMODULE explorerFrame = LoadLibrary(L"ExplorerFrame.dll");

	if (explorerFrame)
	{
		char* CanApplyOwnerDrawToMenu = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 33 DB 48 8B F2 33 FF 48 8B E9";
		char* CAODTMPattern = (char*)FindPattern((uintptr_t)explorerFrame, CanApplyOwnerDrawToMenu);

		if (CAODTMPattern) // 24H2 and later
		{
			ChangeImportedPattern(CAODTMPattern, bytes, sizeof(bytes));
		}
		else
		{
			char* CanApplyOwnerDrawToMenu = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B E9 33 FF 33 D2 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? C7 44 24 20 50 00 00 00";
			char* CAODTMPattern = (char*)FindPattern((uintptr_t)explorerFrame, CanApplyOwnerDrawToMenu);

			if (CAODTMPattern) // 21H2 to 23H2 (i think... i forgot to comment this properly at the time)
			{
				ChangeImportedPattern(CAODTMPattern, bytes, sizeof(bytes));
			}
			else
			{
				char* CanApplyOwnerDrawToMenu = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B E9 33 FF 33 D2";
				char* CAODTMPattern = (char*)FindPattern((uintptr_t)explorerFrame, CanApplyOwnerDrawToMenu);

				if (CAODTMPattern) // TH1 to VB
				{
					ChangeImportedPattern(CAODTMPattern, bytes, sizeof(bytes));
				}
			}
		}
	}
}

// Disabled to prevent the shell from crashing due to incompatibilities with the XAML interface
void DisableWin11AltTab()
{
	if (s_EnableImmersiveShellStack == 1 && g_osVersion.BuildNumber() >= 21996) // only run if we are using Windows 11
	{
		char* ShouldShowMTVAltTab = "40 53 48 83 EC 20 83 79 ?? 02 74 17";
		char* SSMATPattern;
		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };

		HMODULE twinui_pcshell = LoadLibrary(L"twinui.pcshell.dll");

		if (twinui_pcshell)
		{
			SSMATPattern = (char*)FindPattern((uintptr_t)twinui_pcshell, ShouldShowMTVAltTab);
			
			if (SSMATPattern)
			{
				ChangeImportedPattern(SSMATPattern, bytes, sizeof(bytes)); //byebye
			}
		}
	}
}

void DisableWin11HardwareConfirmators()
{
	// Ittr: In simple terms, we disable them because they're unstable and crash-prone
	// Checked against 22631 and 26100
	char* CAudioFlyoutController_Show = "48 83 C1 80 45 33 C9 E9"; // CAudioFlyoutController::Show

	HMODULE twinui = LoadLibrary(L"twinui.dll");

	if (twinui)
	{
		char* CAFCSPattern = (char*)FindPattern((uintptr_t)twinui, CAudioFlyoutController_Show);

		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };

		if (CAFCSPattern)
		{
			ChangeImportedPattern(CAFCSPattern, bytes, sizeof(bytes));
		}
	}
}

void FixWin11SearchIcon()
{
	// Ittr: An accidental change that actually works. Not complaining at all
	// Tested on 22000, 22631 and 26100
	if (g_osVersion.BuildNumber() >= 21996) // build check because this is unnecessary for windows 10
	{
		char* SHIsFileExplorerInTabletMode;
		char* SIFEITMPattern;
		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };

		HMODULE explorerFrame = LoadLibrary(L"ExplorerFrame.dll");

		if (explorerFrame)
		{
			SHIsFileExplorerInTabletMode = "40 55 48 8B EC 48 83 EC 40";
			SIFEITMPattern = (char*)FindPattern((uintptr_t)explorerFrame, SHIsFileExplorerInTabletMode);
			
			if (SIFEITMPattern) // 24H2 and later
			{
				ChangeImportedPattern(SIFEITMPattern, bytes, sizeof(bytes));
			}
			else
			{
				SHIsFileExplorerInTabletMode = "48 89 5C 24 20 55 48 8B EC";
				SIFEITMPattern = (char*)FindPattern((uintptr_t)explorerFrame, SHIsFileExplorerInTabletMode);

				if (SIFEITMPattern) // 21H2 to 23H2
				{
					ChangeImportedPattern(SIFEITMPattern, bytes, sizeof(bytes));
				}
			}
		}
	}
}

void DisableWinXMenu()
{
	// Ittr: This only applies to Windows 10. The menu is no longer part of the immersive shell starting with Windows 11 21H2.
	char* ShowLauncherTipContextMenu; // CImmersiveHotkeyNotification::_ShowLauncherTipContextMenu
	char* SLTCMPattern;
	unsigned char bytes[] = { 0xB0, 0x01, 0xC3 };

	HMODULE twinui = LoadLibrary(L"twinui.dll");

	if (twinui)
	{
		ShowLauncherTipContextMenu = "48 89 5C 24 08 57 48 83 EC 20 48 83 64 24 40 00 48 8B D9 48 8D ?? ?? ?? E8 ?? ?? ?? ?? 45 33 C0"; 
		SLTCMPattern = (char*)FindPattern((uintptr_t)twinui, ShowLauncherTipContextMenu);

		if (SLTCMPattern) // first run, TH1 to TH2
		{
			ChangeImportedPattern(SLTCMPattern, bytes, sizeof(bytes));
		}
		else
		{
			ShowLauncherTipContextMenu = "40 53 48 83 EC 20 48 83 64 24 40 00 48 8B D9 48 8D ?? ?? ?? E8 ?? ?? ?? ?? 45 33 C0";
			SLTCMPattern = (char*)FindPattern((uintptr_t)twinui, ShowLauncherTipContextMenu);

			if (SLTCMPattern) // second run, RS1 to RS5
			{
				ChangeImportedPattern(SLTCMPattern, bytes, sizeof(bytes));
			}
			else
			{
				ShowLauncherTipContextMenu = "40 53 48 83 EC 30 48 C7 44 24 20 FE FF FF FF 48 8B D9 48 83 64 24 50 00 48 8D ?? ?? ?? E8 ?? ?? ?? ?? 45 33 C0";
				SLTCMPattern = (char*)FindPattern((uintptr_t)twinui, ShowLauncherTipContextMenu);

				if (SLTCMPattern) // third run, 19H1 to VB
				{
					ChangeImportedPattern(SLTCMPattern, bytes, sizeof(bytes));
				}
			}
		}
	}
}

void FixWin11ContextMenu()
{
	if (g_osVersion.BuildNumber() >= 21996)
	{
		// Ittr: Works and stops startmenu.dll being called
		// This prevents the crashing issue, restoring Windows 10 behaviour
		char* StartDocked_IsPinnedToStart = "48 89 5C 24 18 48 89 54 24 10 48 89 4C 24 08 55 56 57 41 56";

		HMODULE appResolver = LoadLibrary(L"appresolver.dll");
		if (appResolver)
		{
			char* SDIPTSPattern = (char*)FindPattern((uintptr_t)appResolver, StartDocked_IsPinnedToStart);

			if (SDIPTSPattern)
			{
				unsigned char bytes[] = { 0xC3 };
				ChangeImportedPattern(SDIPTSPattern, bytes, sizeof(bytes));
			}
		}
	}
}

void RevertFlyouts()
{
	if (g_osVersion.BuildNumber() >= 10074) // not needed for 8.1
	{
		if (!s_UseDCompFlyouts || !s_EnableImmersiveShellStack)
		{
			////// VOLUME FLYOUT
			char* LaunchSndVol = "0F 1F 44 00 00 83 FB 66 75 11";

			HMODULE SVS = LoadLibrary(L"SndVolSSO.dll");
			if (SVS) // only run if DLL is present
			{
				char* LSVPattern = (char*)FindPattern((uintptr_t)SVS, LaunchSndVol);

				if (LSVPattern) // first run, VB to GE
				{
					unsigned char bytes[] = { 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x83, 0xFB, 0x66, 0xEB, 0x11 };
					ChangeImportedPattern(LSVPattern, bytes, sizeof(bytes));
				}
				else
				{
					LaunchSndVol = "0F B7 44 24 50 66 83 F8 66 75";

					LSVPattern = (char*)FindPattern((uintptr_t)SVS, LaunchSndVol);

					if (LSVPattern) // second run, TH1 to 19H1
					{
						unsigned char bytes[] = { 0x0F, 0xB7, 0x44, 0x24, 0x50, 0x66, 0x83, 0xF8, 0x66, 0xEB };
						ChangeImportedPattern(LSVPattern, bytes, sizeof(bytes));
					}
				}
			}

			////// NETWORK FLYOUT - See MinHookImports.h
			////// BATTERY FLYOUT - TODO
		}
	}
}

void ChangePatternImports()
{
	// Remove Windows 8+ animation msstyle classes so that legacy msstyles from Vista onwards are compatible with our theming system
	RemoveLoadAnimationDataMap();
	RemoveGetClassIdForShellTarget();

	// Responsible for fixing CLogoffOptions
	FixAuthUI();
	
	// Disable various unwanted immersive interfaces
	DisableImmersiveStart(); // Remove Windows 10+ immersive start menu for UWP mode (doesn't fix hotkeys yet)
	DisableImmersiveSearch(); // Remove Windows 10+ immersive search menu for UWP mode
	DisableTaskView(); // Remove Windows 10+ virtual desktops functionality for UWP mode
	RestoreWin32Menus(); // Remove the immersive menu leftovers so that the taskbar behaves properly in accordance with Windows 7
	DisableWin11AltTab(); // Disable XAML UI because it crashes
	DisableWin11HardwareConfirmators(); // Disable XAML UI because it crashes
	FixWin11SearchIcon(); // Prevents search icon from being mangled by a buggy tablet mode implementation (cheers Microsoft)
	DisableWinXMenu(); // Remove Windows 10 Win+X menu functionality for UWP mode

	// Fix context menus for executable files starting in Windows 11 to prevent explorer from freezing
	FixWin11ContextMenu();

	// Revert flyouts to non-DComp versions on non-UWP or when the user has selected to do this
	RevertFlyouts();
}