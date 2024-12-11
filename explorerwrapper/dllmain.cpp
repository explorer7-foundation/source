#define INITGUID
#define PRERELEASE_COPY
#include "framework.h"
#include "forwards.h"
#include "startmenuresolver.h"
#include "systraywrapper.h"
#include "dbgprint.h"
#include "immersiveshell.h"
#include "traynotify.h"
#include "authui.h"
#include "startmenupin.h"
#include "immersivefactory.h"
#include "projection.h"
#include "version.h"
#include "pinnedlist.h"
#include "destinationlist.h"
#include "resource.h"
#include "thememanager.h"
#include "MinHook.h"
#include "taskscheduler.h"
#include "registry.h"
#include "nsctree.h"
#include "cregtree.h"
#include "shellapi.h"
#include "autoplay.h"
#include "shellitemfilter.h"
#include "shell32_wrappers.h"
#include "shellurl.h"

BOOL g_alttabhooked;
HWND hwnd_desktop;
HWND hwnd_taskbar;
HWND hwnd_startmenu;
HWND hwnd_taskthumb;
HWND hwnd_taskman;
HINSTANCE g_hInstance;
DWORD dwRegisterNotify;
HANDLE hEvent_DesktopVisible;

DWORD g_dwTrayThreadId = 0;

bool g_bClassicTheme = false;
bool g_bDisableComposition = false;
bool g_bEnableImmersiveShellStack = false;
bool g_bRPEnabled = false; // Lol
bool g_bAcrylicAlt = false;
int g_bColorizationOptions = 0;
bool g_bOverrideAlpha = false;
DWORD g_bAlphaValue = 0x6B;

static WNDPROC g_prevTrayProc;
static WNDPROC g_prevThumbnailProc;
typedef DWORD(WINAPI* SHPtrParamAPI)(PVOID);
typedef PVOID(WINAPI* SHCreateDesktopAPI)(PVOID);

static SHCreateDesktopAPI SHCreateDesktopOrig;
static SHPtrParamAPI DwmGetColorizationParametersOrig;
static SHCreateDesktopAPI SHDesktopMessageLoop; //TEST

typedef HWND(WINAPI* CreateWindowInBandAPI)(DWORD, LPWSTR, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, DWORD);
static CreateWindowInBandAPI CreateWindowInBandOrig;

typedef HWND(WINAPI* CreateWindowInBandExAPI)(DWORD, LPWSTR, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, PVOID, DWORD, DWORD);
static CreateWindowInBandExAPI CreateWindowInBandExOrig;

typedef BOOL(WINAPI* GetWindowBandAPI)(HWND, DWORD*);
static GetWindowBandAPI GetWindowBandOrig;

typedef HWND(WINAPI* SetWindowBandApi)(HWND hwnd, HWND hwndInsertAfter, DWORD dwBand);
static SetWindowBandApi SetWindowBandApiOrg;

typedef BOOL(WINAPI* RegisterHotKeyApi)(HWND hwnd, int id, UINT fsMod, UINT vk);
static RegisterHotKeyApi RegisterHotKeyApiOrg;

typedef LONG(WINAPI* GetClassIconCB_t)(PVOID pThis, PVOID a2, int a3);
static GetClassIconCB_t GetClassIconCB_orig;

typedef LONG(WINAPI* setIcon_t)(PVOID pThis, HWND a2, HICON a3, int a4);
static setIcon_t SetIcon;

typedef VOID(WINAPI* updateItem_t)(PVOID pThis, int a2);
static updateItem_t UpdateItem;

typedef VOID(WINAPI* renderThumbnail_t)(PVOID pThis, int, int);
static renderThumbnail_t renderThumbnail_orig;

typedef LONG(WINAPI* setIconThumb_t)(PVOID pThis, HICON a2, int a3, unsigned int a4);
static  setIconThumb_t SetIconThumb;

wiktorArray<HTHEME>* themeHandles;

// 7 {4376df10-a662-420b-b30d-958881461ef9}
// 8 {7A5FCA8A-76B1-44C8-A97C-E7173CCA5F4F}
DEFINE_GUID(IID_TrayClock7, 0x4376df10, 0xa662, 0x420b, 0xb3, 0x0d, 0x95, 0x88, 0x81, 0x46, 0x1e, 0xf9);
DEFINE_GUID(IID_TrayClock8, 0x7A5FCA8A, 0x76B1, 0x44C8, 0xA9, 0x7C, 0xE7, 0x17, 0x3C, 0xCA, 0x5F, 0x4F);

enum ACCENT_STATE : INT {				// Affects the rendering of the background of a window. These names are only for ACCENT_POLICY purposes 
	ACCENT_DISABLED = 0,					// Default value. Background is black.
	ACCENT_ENABLE_GRADIENT = 1,				// Background is GradientColor, alpha channel ignored.
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,	// Background is GradientColor.
	ACCENT_ENABLE_BLURBEHIND = 3,			// Background is GradientColor, with blur effect.
	ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,	// Background is GradientColor, with acrylic blur effect.
	ACCENT_ENABLE_HOSTBACKDROP = 5,			// Unknown.
	ACCENT_INVALID_STATE = 6				// Unknown. Seems to draw background fully transparent.
};

struct ACCENT_POLICY {			// Determines how a window's background is rendered.
	ACCENT_STATE	AccentState;	// Background effect.
	UINT			AccentFlags;	// Flags. Set to 2 to tell GradientColor is used, rest is unknown.
	COLORREF		GradientColor;	// Background color.
	LONG			AnimationId;	// Unknown
};

enum WINDOWCOMPOSITIONATTRIB : INT {	// Determines what attribute is being manipulated.
	WCA_UNDEFINED = 0,
	WCA_NCRENDERING_ENABLED = 1,
	WCA_NCRENDERING_POLICY = 2,
	WCA_TRANSITIONS_FORCEDISABLED = 3,
	WCA_ALLOW_NCPAINT = 4,
	WCA_CAPTION_BUTTON_BOUNDS = 5,
	WCA_NONCLIENT_RTL_LAYOUT = 6,
	WCA_FORCE_ICONIC_REPRESENTATION = 7,
	WCA_EXTENDED_FRAME_BOUNDS = 8,
	WCA_HAS_ICONIC_BITMAP = 9,
	WCA_THEME_ATTRIBUTES = 10,
	WCA_NCRENDERING_EXILED = 11,
	WCA_NCADORNMENTINFO = 12,
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
	WCA_VIDEO_OVERLAY_ACTIVE = 14,
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
	WCA_DISALLOW_PEEK = 16,
	WCA_CLOAK = 17,
	WCA_CLOAKED = 18,
	WCA_ACCENT_POLICY = 19,
	WCA_FREEZE_REPRESENTATION = 20,
	WCA_EVER_UNCLOAKED = 21,
	WCA_VISUAL_OWNER = 22,
	WCA_HOLOGRAPHIC = 23,
	WCA_EXCLUDED_FROM_DDA = 24,
	WCA_PASSIVEUPDATEMODE = 25,
	WCA_USEDARKMODECOLORS = 26,
	WCA_CORNER_STYLE = 27,
	WCA_PART_COLOR = 28,
	WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
	WCA_SYSTEMBACKDROP_TYPE = 30,
	WCA_SET_TAGGED_WINDOW_RECT = 31,
	WCA_CLEAR_TAGGED_WINDOW_RECT = 32,
	WCA_LAST = 33
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
	WINDOWCOMPOSITIONATTRIB Attrib; // the attribute to query, see below
	void* pvData; // buffer to store the result
	UINT cbData; // size of the pData buffer
};

typedef BOOL(WINAPI* SetWindowCompositionAttributeAPI) (HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA* pAttrData);
static SetWindowCompositionAttributeAPI SetWindowCompositionAttribute;

typedef HRESULT(WINAPI* DwmpUpdateAccentBlurRect_t)(HWND, LPRECT);
static DwmpUpdateAccentBlurRect_t DwmpUpdateAccentBlurRect;

//////////////// WITH THANKS AND CREDITS TO EXPLORERPATCHER ////////////////
typedef enum IMMERSIVE_COLOR_TYPE
{
	// Defining only used ones
	IMCLR_SystemAccentLight2 = 0x2,
	IMCLR_SystemAccentDark2 = 0x6
} IMMERSIVE_COLOR_TYPE;

typedef struct IMMERSIVE_COLOR_PREFERENCE
{
	DWORD crStartColor;
	DWORD crAccentColor;
} IMMERSIVE_COLOR_PREFERENCE;

typedef enum IMMERSIVE_HC_CACHE_MODE
{
	IHCM_USE_CACHED_VALUE = 0,
	IHCM_REFRESH = 1
} IMMERSIVE_HC_CACHE_MODE;

typedef void(*GetThemeName_t)(void*, void*, void*); // 74
GetThemeName_t GetThemeName;

typedef bool(*RefreshImmersiveColorPolicyState_t)(); // 104
RefreshImmersiveColorPolicyState_t RefreshImmersiveColorPolicyState;

typedef bool(*GetIsImmersiveColorUsingHighContrast_t)(IMMERSIVE_HC_CACHE_MODE); // 106
GetIsImmersiveColorUsingHighContrast_t GetIsImmersiveColorUsingHighContrast;

typedef HRESULT(*GetUserColorPreference_t)(IMMERSIVE_COLOR_PREFERENCE*, bool); // 120
GetUserColorPreference_t GetUserColorPreference;

typedef DWORD(*GetColorFromPreference_t)(const IMMERSIVE_COLOR_PREFERENCE*, IMMERSIVE_COLOR_TYPE, bool, IMMERSIVE_HC_CACHE_MODE); // 121
GetColorFromPreference_t GetColorFromPreference;

class CImmersiveColor
{
public:
	static DWORD GetColor(IMMERSIVE_COLOR_TYPE colorType)
	{
		IMMERSIVE_COLOR_PREFERENCE icp;
		icp.crStartColor = 0;
		icp.crAccentColor = 0;
		GetUserColorPreference(&icp, true/*, true*/);
		return GetColorFromPreference(&icp, colorType, true, IHCM_REFRESH);
	}

	static bool IsColorSchemeChangeMessage(UINT uMsg, LPARAM lParam)
	{
		bool bRet = false;
		if (uMsg == WM_SETTINGCHANGE && lParam && CompareStringOrdinal((WCHAR*)lParam, -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
		{
			RefreshImmersiveColorPolicyState();
			bRet = true;
		}
		GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
		return bRet;
	}
};

class CImmersiveColorImpl
{
public:
	static HRESULT GetColorPreferenceImpl(IMMERSIVE_COLOR_PREFERENCE* pcpPreference, bool fForceReload, bool fUpdateCached)
	{
		return GetUserColorPreference(pcpPreference, fForceReload);
	}
};
//////////////// END WITH THANKS AND CREDITS TO EXPLORERPATCHER ////////////////

//extern declared in nsctree.h
HRESULT(__fastcall* CNSCHost_FillNSCOg)(uintptr_t nscHost);

//extern declared in version.h
COSVersion g_osVersion;

const LPWSTR sz_DesktopWindowManagerKey = L"SOFTWARE\\Microsoft\\Windows\\DWM"; // Ittr: used for colorization fix by force
const LPWSTR sz_SettingsKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"; // already defined in registry.cpp, should really be accessed better sorry! intention to remove this one after m2 and we officially support win11.
const LPWSTR sz_ShellFolder = L"SOFTWARE\\Classes\\CLSID\\{865e5e76-ad83-4dca-a109-50dc2113ce9a}"; // used for shellfolder creation
const LPWSTR sz_ShellFolder2 = L"SOFTWARE\\Classes\\CLSID\\{865e5e76-ad83-4dca-a109-50dc2113ce9a}\\InProcServer32"; // used for shellfolder creation
const LPWSTR sz_ShellFolder3 = L"SOFTWARE\\Classes\\CLSID\\{865e5e76-ad83-4dca-a109-50dc2113ce9a}\\ShellFolder"; // used for shellfolder creation

static LRESULT RegGetDWORD(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	DWORD sz = 4;
	return SHRegGetValueW(key, subkey, value, SRRF_RT_REG_DWORD, NULL, dwVal, &sz);
}

static LRESULT RegSetDWORD(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	return SHSetValueW(key, subkey, value, REG_DWORD, dwVal, 4);
}

static LRESULT RegSetSZ(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	return SHSetValueW(key, subkey, value, REG_SZ, dwVal, (DWORD)wcslen((wchar_t*)dwVal) * sizeof(dwVal[0]));
}

static LRESULT RegSetExpandSZ(HKEY key, LPWSTR subkey, LPWSTR value, DWORD* dwVal)
{
	return SHSetValueW(key, subkey, value, REG_EXPAND_SZ, dwVal, 2 * ((DWORD)wcslen((wchar_t*)dwVal) * sizeof(dwVal[0])));
}

typedef struct {
	DWORD ColorizationColor;
	DWORD ColorizationAfterglow;
	DWORD ColorizationColorBalance;
	DWORD ColorizationAfterglowBalance;
	DWORD ColorizationBlurBalance;
	DWORD ColorizationGlassReflectionIntensity;
	DWORD ColorizationOpaqueBlend;
} DWMCOLORIZATIONPARAMS, * PDWMCOLORIZATIONPARAMS;

static BOOL IsRTMDWM()
{
	if (!IsCompositionActive()) return FALSE;

	DWMCOLORIZATIONPARAMS colors;
	CHAR buffer[0x28];
	memset(buffer, 0, 0x28);
	DwmGetColorizationParametersOrig(&buffer);
	memcpy(&colors, (PVOID)buffer, sizeof(DWMCOLORIZATIONPARAMS));
	return (colors.ColorizationGlassReflectionIntensity == 1);
}

static HWND GetTaskbarWnd()
{
	if (!hwnd_taskbar)
		hwnd_taskbar = FindWindow(L"Shell_TrayWnd", NULL);
	return hwnd_taskbar;
}

static BOOL CALLBACK FindSMCallback(HWND hwnd, LPARAM lParam)
{
	if (GetClassWord(hwnd, GCW_ATOM) == (ATOM)lParam && (GetProp(hwnd, L"StartMenuTag")))
	{
		hwnd_startmenu = hwnd;
		return FALSE;
	}
	return TRUE;
}

static HWND GetStartMenuWnd()
{
	if (!hwnd_startmenu || !IsWindow(hwnd_startmenu))
	{
		WNDCLASS dummy = { 0 };
		ATOM dv2atom = GetClassInfo(GetModuleHandle(NULL), L"DV2ControlHost", &dummy);
		EnumThreadWindows(GetCurrentThreadId(), FindSMCallback, (LPARAM)dv2atom);
	}
	return hwnd_startmenu;
}

static HWND GetThumbnailWnd()
{
	if (!hwnd_taskthumb)
		hwnd_taskthumb = FindWindow(L"TaskListThumbnailWnd", NULL);
	return hwnd_taskthumb;
}

const UINT ThemeChangeMessage = WM_USER + 69420;
BOOL CALLBACK RefreshWindows(HWND wnd, LPARAM prm)
{
	if (wnd == (HWND)prm) return TRUE;

	PostMessage(wnd, WM_THEMECHANGED, 0, 0);
	dbgprintf(L"themechanged sent to %i", wnd);
	return TRUE;
}

// Ittr: Forcing this change fixes colorization on aero.msstyles for 1809+ on taskbar and start menu ONLY.
void EnsureWindowColorization()
{
	if (g_osVersion.BuildNumber() >= 17763)
	{
		DWORD value = 0; // initialise in memory
		DWORD colorVal = 1; // doesn't work when reduced to a single string, annoying but atleast we can use it here
		RegGetDWORD(HKEY_CURRENT_USER, sz_DesktopWindowManagerKey, L"EnableWindowColorization", &value); // output the data from attributes key...

		if (value != colorVal) // basically if the attribute value doesn't exist or is the wrong value...
		{
			RegSetDWORD(HKEY_CURRENT_USER, sz_DesktopWindowManagerKey, L"EnableWindowColorization", &colorVal); // apply folder attributes, arguably the most important part
		}
	}
}

DWORD GetColorizationColor()
{
	DWMCOLORIZATIONPARAMS colors;
	CHAR buffer[0x28];
	memset(buffer, 0, 0x28);
	DwmGetColorizationParametersOrig(&buffer);
	memcpy(&colors, (PVOID)buffer, sizeof(DWMCOLORIZATIONPARAMS));

	int a = (colors.ColorizationColor >> 24) & 0xFF;
	int r = (colors.ColorizationColor >> 16) & 0xFF;
	int g = (colors.ColorizationColor >> 8) & 0xFF;
	int b = (colors.ColorizationColor) & 0xFF;

	// thanks to microsoft we have to account for automatic colorization being bugged on 10+ as alpha is set to 0. Yay...
	if (g_osVersion.BuildNumber() >= 10074 && g_bColorizationOptions != 3 && a == 0x00 && (r != 0x00 || g != 0x00 || b != 0x00)) // only apply if it appears that the user is trying to set an actual colour - full transparency remains possible!
		a = 0xC4; // we default to this as it's used by the majority of win10/11 default colours

	if (g_bColorizationOptions == 4) // mode 4 (gradient non-transparent is buggy) + current thumbnail edge case 
		a = 0xFF;

	// Windows 10 and 11 users specifically without glass tools may struggle to adjust color opacity, this optional override fixes this
	if (g_bOverrideAlpha && (g_bColorizationOptions == 1 || g_bColorizationOptions == 2))
		a = (g_bAlphaValue) & 0xFF;

	if (g_bColorizationOptions == 3)
	{
		GetThemeName = (GetThemeName_t)GetProcAddress(LoadLibrary(L"uxtheme.dll"), (LPSTR)74);
		RefreshImmersiveColorPolicyState = (RefreshImmersiveColorPolicyState_t)GetProcAddress(LoadLibrary(L"uxtheme.dll"), (LPSTR)104);
		GetIsImmersiveColorUsingHighContrast = (GetIsImmersiveColorUsingHighContrast_t)GetProcAddress(LoadLibrary(L"uxtheme.dll"), (LPSTR)106);
		GetUserColorPreference = (GetUserColorPreference_t)GetProcAddress(LoadLibrary(L"uxtheme.dll"), (LPSTR)120);
		GetColorFromPreference = (GetColorFromPreference_t)GetProcAddress(LoadLibrary(L"uxtheme.dll"), (LPSTR)121);
	}

	DWORD color = (g_bColorizationOptions != 3) ? ((a << 24) | (b << 16) | (g << 8) | r) : (0xCC000000 | (CImmersiveColor::GetColor((g_bAcrylicAlt == 1) ? IMCLR_SystemAccentLight2 : IMCLR_SystemAccentDark2) & 0xFFFFFF));
	return color;
}

ACCENT_STATE GetAccentState(bool isThumbnail)
{
	if (g_bColorizationOptions == 3) // acrylic (1803-)
		return ACCENT_ENABLE_ACRYLICBLURBEHIND;
	else if (g_bColorizationOptions == 2) // blurbehind (1507 until 11 21h2)
		return ACCENT_ENABLE_BLURBEHIND;

	if (isThumbnail) // run this block after the other ones, to ensure that pseudo-aero mode uses opaque thumbnail. using the option definition causes extreme visual bugs for some reason.
		return ACCENT_ENABLE_GRADIENT;

	// pseudo-aero & solid-color (all versions) - the replacements for option 0 & fallback for other values entered > 4
	return ACCENT_ENABLE_TRANSPARENTGRADIENT; // we use transparentgradient for both 1 and 4, as gradient has some weird hrgn side-effects on start menu

}

WINDOWCOMPOSITIONATTRIBDATA GetTrayAccentProperties(bool isThumbnail)
{
	// to break down what happens here:
	// - we create an accent policy
	// - we take in whether thumbnail wnd is calling so we can make tweaks as needed
	// - accent state gets calculated and returned based on user preference
	// - this calculation is tweaked if thumbnail wnd flag is passed in, as pseudo-aero looks better with solid thumbnail
	// - we then define the accent flags - start menu and taskbar work fine with 0x13, thumbnail requires 0x200 to work properly on later windows 10 versions (presumably OK on earlier vers)
	// - 0x200 is then added to with extra to ensure that blurbehind mode takes in color properly
	// - we then define gradient color by pulling either DWM accent color or immersive color as applicable
	// this is then passed into attribute data which we call back into whenever we need to get accent properties without retyping this whole function

	WINDOWCOMPOSITIONATTRIBDATA attrData;
	ACCENT_POLICY accentPolicy;

	accentPolicy.AccentState = GetAccentState(isThumbnail);
	accentPolicy.AccentFlags = (isThumbnail) ? (0x1 | 0x2 | 0x200) : (0x13); // very important that this is set up like this!
	accentPolicy.GradientColor = GetColorizationColor();

	attrData.Attrib = WCA_ACCENT_POLICY;
	attrData.pvData = &accentPolicy;
	attrData.cbData = sizeof(accentPolicy);
	return attrData;
}

LRESULT CALLBACK NewTrayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // Ittr: for TH1+
		SetProgmanAsShell(); // misha: TODO hack

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
		if ((IsThemeActive() && !g_bClassicTheme && IsCompositionActive() && !g_bDisableComposition) && hwnd == GetTaskbarWnd() && g_bColorizationOptions != 0) // Ittr: Only taskbar needs updating now, start menu and new thumbnail algo correct for themselves
			SetWindowCompositionAttribute(hwnd, &GetTrayAccentProperties(false));

	}

	return CallWindowProc(g_prevTrayProc, hwnd, uMsg, wParam, lParam);
}

// Ittr: Awful hack but it seems to fix it
LRESULT CALLBACK NewThumbnailProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SETTINGCHANGE || uMsg == WM_ERASEBKGND || uMsg == WM_WININICHANGE) // Ittr: Fix thumbnail colorization for non-legacy
	{
		if ((IsThemeActive() && !g_bClassicTheme && IsCompositionActive() && !g_bDisableComposition) && (g_osVersion.BuildNumber() >= 10074 && hwnd == GetThumbnailWnd()) && g_bColorizationOptions != 0) // Ittr: Only taskbar needs updating now, start menu and new thumbnail algo correct for themselves
			SetWindowCompositionAttribute(hwnd, &GetTrayAccentProperties(true));

	}

	return CallWindowProc(g_prevThumbnailProc, hwnd, uMsg, wParam, lParam);
}

void ShimDesktop8()
{
	static int InitOnce = FALSE;
	if (InitOnce) return;
	hwnd_desktop = FindWindow(L"Progman", L"Program Manager");
	HWND hwndTray = GetTaskbarWnd();
	HWND hwndThumbnail = GetThumbnailWnd();
	if (!hwnd_desktop || !hwndTray || !hwndThumbnail) return;
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
	ShimDesktop8();
	return ret;
}

PVOID WINAPI SHDesktopMessageLoopNEW(PVOID p1)
{
	PVOID ret = SHDesktopMessageLoop(p1);
	//SHPtrParamAPI SHCloseDesktopHandle;
	//SHCloseDesktopHandle = (SHPtrParamAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"),(LPSTR)206);
	//SHCloseDesktopHandle(p1);
	return ret;
}

DWORD WINAPI DwmGetColorizationParametersNEW(PDWMCOLORIZATIONPARAMS colors)
{
	CHAR buffer[0x28];
	memset(buffer, 0, 0x28);
	dbgprintf(L"DwmGetColorizationParameters\nColorizationColor %p\nColorizationAfterglow %p\nColorizationColorBalance %p\nColorizationAfterglowBalance %p\nColorizationBlurBalance %p\nColorizationGlassReflectionIntensity %p\nColorizationOpaqueBlend %p",
		colors->ColorizationColor, colors->ColorizationAfterglow, colors->ColorizationColorBalance, colors->ColorizationAfterglowBalance, colors->ColorizationBlurBalance, colors->ColorizationGlassReflectionIntensity, colors->ColorizationOpaqueBlend);
	DWORD ret = DwmGetColorizationParametersOrig(&buffer);
	memcpy(colors, (PVOID)buffer, sizeof(DWMCOLORIZATIONPARAMS));
	return ret;
}

//Ittr: Intercept these functions where appropriate for basic theme to be forced at compile time if required
BOOL WINAPI IsCompositionActiveNEW()
{
	if (g_bDisableComposition) { return FALSE; }

	return IsCompositionActive();
}

HRESULT WINAPI DwmIsCompositionEnabledNEW(BOOL* pfEnabled)
{
	if (g_bDisableComposition) { return 0x80263001; } //0x80263001 is the value to signify composition being disabled for some reason

	return DwmIsCompositionEnabled(pfEnabled);
}

//Ittr: Less lines of code and more utility/reusability for setting composition attributes in future
void ForceActiveWindowAppearance(HWND hwnd)
{
	BOOL bForceActiveWindowAppearance = true;
	WINDOWCOMPOSITIONATTRIBDATA attrData;
	attrData.Attrib = WCA_FORCE_ACTIVEWINDOW_APPEARANCE;
	attrData.pvData = &bForceActiveWindowAppearance;
	attrData.cbData = sizeof(bForceActiveWindowAppearance);
	SetWindowCompositionAttribute(hwnd, &attrData);
}

BOOL WINAPI SetWindowCompositionAttributeNEW(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA* pAttrData) // Ittr: re-organised again 25/10/24
{
	dbgprintf(L"SetWindowCompositionAttribute %X %x %d", hwnd, pAttrData->Attrib, *(DWORD*)pAttrData->pvData);

	if (!IsThemeActive() || g_bClassicTheme || !IsCompositionActive() || g_bDisableComposition) // we do funny things so explorer works properly for classic/basic.
	{
		int bNCRenderingEnabled = DWMNCRP_DISABLED;

		// Disable DWM frames
		WINDOWCOMPOSITIONATTRIBDATA attrData;
		attrData.Attrib = WCA_NCRENDERING_POLICY;
		attrData.pvData = &bNCRenderingEnabled;
		attrData.cbData = sizeof(bNCRenderingEnabled);

		SetWindowCompositionAttribute(hwnd, &attrData); //byebye
		return SetWindowCompositionAttribute(hwnd, pAttrData);
	}

	if (IsCompositionActiveNEW() && pAttrData->Attrib == WCA_DISALLOW_PEEK) // if user has DWM enabled, and is not using basic/classic
	{
		if (g_bColorizationOptions != 0 && (hwnd == GetTaskbarWnd() || hwnd == GetStartMenuWnd() || (g_osVersion.BuildNumber() >= 10074 && hwnd == GetThumbnailWnd()))) // for pseudo-aero, blurbehind, acrylic & solid modes
			SetWindowCompositionAttribute(hwnd, &GetTrayAccentProperties((hwnd == GetThumbnailWnd()) ? true : false));

		ForceActiveWindowAppearance(hwnd); // mainly for legacy but doesn't seem to harm anything by applying anyway
	}

	return SetWindowCompositionAttribute(hwnd, pAttrData);
}

HRESULT WINAPI DwmEnableBlurBehindWindowNEW(HWND hwnd, DWM_BLURBEHIND* pBlurBehind)
{
	if (hwnd == GetThumbnailWnd()) // does this even do anything??
		ForceActiveWindowAppearance(hwnd);

	if ( IsRTMDWM() && (hwnd == GetTaskbarWnd() || hwnd == GetStartMenuWnd() || (g_osVersion.BuildNumber() >= 10074 && hwnd == GetThumbnailWnd())) && g_bColorizationOptions != 0) //enable rtm pseudo-aero
		pBlurBehind->fEnable = 0;
	return DwmEnableBlurBehindWindow(hwnd, pBlurBehind);
}

int WINAPI SetWindowRgnNEW(HWND hwnd, HRGN hRgn, BOOL bRedraw)
{
	//don't allow to reset start menu rgn - rtm pseudo aero glitches
	// TODO in future: more sophisticated RGN fixes so this isn't necessary?
	if (hRgn == NULL && hwnd == GetStartMenuWnd() && g_bColorizationOptions > 0) return 0;
	return SetWindowRgn(hwnd, hRgn, bRedraw);
}

HRESULT WINAPI SetWindowThemeNEW(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	// hook for enabling w8 theme classes. this is the limit of this feature due to backlash
	if (g_bRPEnabled)
	{
		if (lstrcmp(pszSubAppName, L"VerticalShowDesktop") == 0) return SetWindowTheme(hwnd, L"VerticalShowDesktop8", pszSubIdList);
		if (lstrcmp(pszSubAppName, L"ShowDesktop") == 0) return SetWindowTheme(hwnd, L"ShowDesktop8", pszSubIdList);

		if (hwnd == GetThumbnailWnd() && (lstrcmp(pszSubAppName, L"Vertical") != 0) && IsCompositionActiveNEW()) // updated thumbnail classes misbehave without DWM
			return SetWindowTheme(hwnd, L"W8", pszSubIdList);
		else if (hwnd == GetThumbnailWnd() && (lstrcmp(pszSubAppName, L"Vertical") == 0) && IsCompositionActiveNEW())
			return SetWindowTheme(hwnd, L"W8Vertical", pszSubIdList);

	}

	return SetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

UINT WINAPI SetErrorModeNEW(UINT uMode)
{
	SetCurrentProcessExplicitAppUserModelID(L"Microsoft.Windows.Explorer");

	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074)
		CreateTwinUI_UWP();

	return SetErrorMode(uMode);
}

typedef BOOL(WINAPI* IsShellWindow_t)(HWND);
IsShellWindow_t IsShellFrameWindow = nullptr;
//IsShellWindow_t IsShellManagedWindow = nullptr;

typedef HWND(WINAPI* GhostWindowFromHungWindow_t)(HWND);
GhostWindowFromHungWindow_t GhostWindowFromHungWindow = nullptr;

ATOM g_SecondaryTaskbarAtom;

HWND* v_hwndDesktop;

BOOL ShouldAddWindowToTrayHelper(HWND hwnd)
{
	//if (Feature_WindowTabHost && !IsValidTabWindowForTray(hwnd))
	//	return FALSE;

	DWORD dwExStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
	return (!GetWindow(hwnd, GW_OWNER) || (dwExStyle & WS_EX_APPWINDOW) != 0) && (dwExStyle & WS_EX_TOOLWINDOW) == 0;
}

namespace ShellManagedWindowHelper
{
	bool ShouldTreatShellManagedWindowAsNotShellManaged(HWND hwnd)
	{
		return GetPropW(hwnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") != 0 || GetPropW(hwnd, L"Windows.ImmersiveShell.DisableShowingMainViewOnActivation") == 0;
	}
}

enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

struct BandData
{
	ZBID id;
	bool bInclude;
};

static const BandData s_bandInclusionData[] =
{
	{ ZBID_DEFAULT, false },
	{ ZBID_DESKTOP, true },
	{ ZBID_UIACCESS, true },
	{ ZBID_IMMERSIVE_IHM, false },
	{ ZBID_IMMERSIVE_NOTIFICATION, false },
	{ ZBID_IMMERSIVE_APPCHROME, false },
	{ ZBID_IMMERSIVE_MOGO, false },
	{ ZBID_IMMERSIVE_EDGY, false },
	{ ZBID_IMMERSIVE_INACTIVEMOBODY, false },
	{ ZBID_IMMERSIVE_INACTIVEDOCK, false },
	{ ZBID_IMMERSIVE_ACTIVEMOBODY, false },
	{ ZBID_IMMERSIVE_ACTIVEDOCK, false },
	{ ZBID_IMMERSIVE_BACKGROUND, false },
	{ ZBID_IMMERSIVE_SEARCH, false },
	{ ZBID_GENUINE_WINDOWS, false },
	{ ZBID_IMMERSIVE_RESTRICTED, false },
	{ ZBID_SYSTEM_TOOLS, true },
	{ ZBID_LOCK, false },
	{ ZBID_ABOVELOCK_UX, false }
};

BOOL WINAPI GetWindowBandNew(HWND hwnd, DWORD* out);

BOOL __stdcall GetWindowBandHelper(HWND hwnd, ZBID* out)
{
	if (GetWindowBandOrig)
	{
		return GetWindowBandNew(hwnd, (DWORD*)out);
	}

	static BOOL(__stdcall * fn)(HWND, ZBID*) = nullptr;
	if (!fn)
	{
		HMODULE h = GetModuleHandleW(L"user32.dll");
		if (h)
			fn = (decltype(fn))GetProcAddress(h, "GetWindowBand");
		//FAIL_FAST_IF_NULL(fn);
		if (!fn)
			return 0;
	}
	return fn(hwnd, out);
}


typedef BOOL(*IsShellManagedWindow_t)(HWND hwnd); // 2574
BOOL IsShellManagedWindow(HWND hwnd)
{
	static IsShellManagedWindow_t fn = nullptr;
	if (!fn)
	{
		HMODULE h = GetModuleHandleW(L"user32.dll");
		if (h)
			fn = (IsShellManagedWindow_t)GetProcAddress(h, MAKEINTRESOURCEA(2574));
		//FAIL_FAST_IF_NULL(fn);
		if (!fn)
			return 0;
	}
	return fn(hwnd);
}

bool ShouldExcludeFromTaskbar(HWND hwnd)
{
	wchar_t text[256];
	GetWindowTextW(hwnd, text, 255);

	if (!StrCmpW(text, L"Microsoft Text Input Application") || !StrCmpW(text, L"Windows Shell Experience Host") || !StrCmpW(text, L"Start") || !StrCmpW(text, L"Search"))
		return true;

	return false;
}

bool IsValidDesktopZOrderBand(HWND hwnd, BOOL bCheckShellManagedWindow)
{
	bool bValid = false;

	ZBID band;
	if (GetWindowBandHelper(hwnd, &band))
	{
		bValid = s_bandInclusionData[band].bInclude;

		//if (Feature_WindowTabHost && (HWND)GetPropW(hwnd, (LPCWSTR)0xA920))
		//	bValid = true;

		if (bValid && bCheckShellManagedWindow)
			bValid = !IsShellManagedWindow(hwnd) || ShellManagedWindowHelper::ShouldTreatShellManagedWindowAsNotShellManaged(hwnd);
	}

	if (bValid)
		bValid = !ShouldExcludeFromTaskbar(hwnd);

	return bValid;
}

bool IsWindowNotDesktopOrTray(HWND hwnd)
{
	if (!IsWindow(hwnd) || !IsValidDesktopZOrderBand(hwnd, TRUE) || hwnd == hwnd_taskbar || (v_hwndDesktop && hwnd == *v_hwndDesktop))
		return false;

	//if (GetClassWord(hwnd, GCW_ATOM) == g_SecondaryTaskbarAtom)
	//	return g_SecondaryTaskbarAtom == 0;

	return true;
}

//removes immersive background windows
//(Microsoft Text Input Host, Shell Experience Host, etc.)
BOOL WINAPI IsWindowVisibleNEW(HWND hWnd)
{
	if (!IsWindowVisible(hWnd) || !IsValidDesktopZOrderBand(hWnd, TRUE))
		return FALSE;

	BOOL bCloaked;
	DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &bCloaked, sizeof(BOOL));
	if (bCloaked)
		return FALSE;

	if (IsShellFrameWindow && GhostWindowFromHungWindow)
	{
		if (IsShellFrameWindow(hWnd) && !GhostWindowFromHungWindow(hWnd))
			return TRUE;
	}

	//if (IsShellManagedWindow)
	{
		if (IsShellManagedWindow(hWnd) && GetPropW(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") == NULL)
			return FALSE;
	}

	return TRUE;
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

__int64 ShouldAddWindowToTray(HWND hwnd)
{
	BOOL ret = IsWindowNotDesktopOrTray(hwnd) && IsWindowVisibleNEW(hwnd) && ShouldAddWindowToTrayHelper(hwnd);
	//dbgprintf(L"ShouldAddWindowToTray %i", (int)ret);
	return ret;
}

__int64(__fastcall* DwmpActivateLivePreview)(int a1, __int64 a2, __int64 a3, int a4, void* a5);
__int64 DwmpActivateLivePreviewNEW(int a1, __int64 a2, __int64 a3, int a4, void* a5)
{
	if (a5 == (void*)8)
		a5 = 0;

	if (a5 && IsBadReadPtr(a5, 0x8))
		a5 = 0;
	return DwmpActivateLivePreview(a1, a2, a3, a4, a5);
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
		//OutputDebugStringW(L"uwp window");
		HICON hc = GetUWPIcon(v6);
		if (hc) SetIconThumb(This, hc, a2, 3);
	}
	else
		UpdateItem(This, a2);

}

PVOID CtaskBandPtr = 0;

VOID SetWindowIcon(PVOID This, HWND a2, HICON a3, int a4)
{
	CtaskBandPtr = This;
	if (IsShellFrameWindow && IsShellFrameWindow(a2))
	{
		HICON hc = GetUWPIcon(a2);
		if (hc) SetIcon(This, a2, hc, a4);
	}
	else
		SetIcon(This, a2, a3, a4);
}

/*
INT64 GetClassIconCB(PVOID This, struct ICONBCPARAM* a2, int a3)
{
	INT64 ret = 0;
	if (IsShellFrameWindow)
	{
		if (IsShellFrameWindow(*(HWND*)a2+1))
		{
			return 4294967294;
			//return GetClassIconCB_orig(This, a2, a3);
		}
		else
		{
			return GetClassIconCB_orig(This, a2, a3);
		}
	}
	return 0;
}
*/

//fix for classic start menu icon
typedef HANDLE(WINAPI* BrandingLoadImage_t)(
	LPCWSTR lpszModule,
	UINT    uImageId,
	UINT    type,
	int     cx,
	int     cy,
	UINT    fuLoad
	);
BrandingLoadImage_t BrandingLoadImage = nullptr;
HANDLE WINAPI BrandingLoadImageNEW(
	LPCWSTR lpszModule,
	UINT    uImageId,
	UINT    type,
	int     cx,
	int     cy,
	UINT    fuLoad
)
{
	WCHAR msg[256];
	wsprintfW(msg, L"BrandingLoadImage, id: %u", uImageId);
	MessageBoxW(NULL, msg, L"debug", NULL);

	UINT uNewId = 0;
	switch (uImageId)
	{
	case 1041:
		uNewId = IDB_START;
		break;
	case 2041:
		uNewId = IDB_START_125;
		break;
	case 3041:
		uNewId = IDB_START_150;
		break;
	}

	if (uNewId)
		return LoadImageW(
			g_hInstance,
			MAKEINTRESOURCE(uNewId),
			type, cx, cy, fuLoad
		);
	else
		return BrandingLoadImage(
			lpszModule, uImageId, type,
			cx, cy, fuLoad
		);
}

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

	DWORD old;
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

	if (pattern1 && !pattern) //Ittr: Only apply to CLogoffPane::_OnCreate if we need to, otherwise this causes crashing on later 7 explorer.
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


int g_fDPIAware = 0;
int g_nScreenDpi = 0;
int g_fForcedDpi = 0;
__int64 GetScreenDpi(void)
{
	int v0; // eax
	HDC DC; // rax
	HDC v3; // rbx

	if (!g_fForcedDpi)
	{
		v0 = IsProcessDPIAware();
		if (g_fDPIAware != v0 || !g_nScreenDpi)
		{
			g_fDPIAware = v0;
			g_nScreenDpi = 96;
			DC = GetDC(0LL);
			v3 = DC;
			if (DC)
			{
				g_nScreenDpi = GetDeviceCaps(DC, 88);
				ReleaseDC(0LL, v3);
			}
		}
	}
	return (unsigned int)g_nScreenDpi;
}

bool IsClassicTheme(void)
{
	return !IsThemeActive() || g_bClassicTheme;
}

bool AllowThemes(void)
{
	return !IsClassicTheme();
}

HTHEME(__stdcall* fOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
HTHEME(__stdcall* fOpenThemeDataForDpi)(HWND hwnd, LPCWSTR pszClassList, UINT dpi);
HTHEME(__stdcall* fOpenThemeDataEx)(HWND hwnd, LPCWSTR pszClassList, DWORD dwFlags);
HTHEME __stdcall OpenThemeData_Hook(HWND hwnd, LPCWSTR pszClassList)
{
	if (g_dwTrayThreadId > 0 && g_dwTrayThreadId != GetCurrentThreadId())
		return fOpenThemeData(hwnd, pszClassList);

	if (!AllowThemes())
		return NULL;

	HTHEME theme = 0;
	DWORD flags = 2;
	if ((unsigned int)GetScreenDpi() != 96)
		flags |= 1u;

	if (g_loadedTheme)
		theme = OpenThemeDataFromFile(g_loadedTheme, hwnd, pszClassList, flags);
	else
		theme = fOpenThemeData(hwnd, pszClassList);

	if (theme == nullptr)
		dbgprintf(L"OPENTHEMEDATA FAILED %s", pszClassList);
	themeHandles->push_back(theme);
	return theme;
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

	if (g_loadedTheme)
		theme = OpenThemeDataFromFile(g_loadedTheme, hwnd, pszClassList, flags);
	else
		theme = fOpenThemeDataForDpi(hwnd, pszClassList, dpi);

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

LPTHREAD_START_ROUTINE CTray__SyncThreadProc_orig = nullptr;
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

/* Adjust a window's position to be pushed away from the taskbar */
SIZE AdjustWindowRectForTaskbar(RECT* lprc)
{
	HMONITOR hm = MonitorFromRect(lprc, MONITOR_DEFAULTTONEAREST);
	HDC hDC = GetDC(NULL);
	int offset = MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 96);
	ReleaseDC(NULL, hDC);

	MONITORINFO mi = { sizeof(MONITORINFO) };
	GetMonitorInfoW(hm, &mi);

	int dx = 0, dy = 0;
	PLONG plrc = (PLONG)lprc;
	PLONG plwrc = (PLONG)&mi.rcWork;
	for (int i = 0; i < 4; i++)
	{
		int curOffset = plwrc[i] - plrc[i];
		curOffset = (curOffset < 0) ? -curOffset : curOffset;

		if (curOffset < offset)
		{
			int* set = (i % 2 == 0) ? &dx : &dy;
			if (i > 1)
			{
				*set -= offset - curOffset;
			}
			else
			{
				*set += offset - curOffset;
			}
		}
	}
	return { dx, dy };
}

BOOL WINAPI CalculatePopupWindowPositionNEW(
	const POINT* anchorPoint,
	const SIZE* windowSize,
	UINT         flags,
	RECT* excludeRect,
	RECT* popupWindowPosition
)
{
	BOOL res = CalculatePopupWindowPosition(
		anchorPoint, windowSize, flags,
		excludeRect, popupWindowPosition
	);
	if (IsCompositionActiveNEW() && res && (flags & TPM_WORKAREA) != 0)
	{
		SIZE adjust = AdjustWindowRectForTaskbar(popupWindowPosition);
		OffsetRect(popupWindowPosition, adjust.cx, adjust.cy);
	}
	return res;
}


HWND WINAPI CreateWindowInBandNew(DWORD dwExStyle,
	LPCWSTR lpClassName,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hwndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam,
	DWORD dwBand)
{
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // UWP enabled
	{
		DWORD p0 = (DWORD)_ReturnAddress();
		dwExStyle = dwExStyle | WS_EX_TOOLWINDOW; // TODO is this needed
		HWND ret = CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hwndParent, hMenu, hInstance, lpParam);

		// Ittr: We do this to eliminate the ghost window. The power of trans rights compelled me to fix this :3
		BOOL shouldCloak = true;
		WCHAR titleBuffer[MAX_PATH];
		GetClassName(ret, titleBuffer, sizeof(titleBuffer));
		WCHAR afwTitle[23] = L"ApplicationFrameWindow";
		if (strcmp((char*)titleBuffer, (char*)afwTitle) == 0)
			DwmSetWindowAttribute(ret, DWMWA_CLOAK, &shouldCloak, sizeof(shouldCloak));

		dbgprintf(L"CREATEWINDOWINBANDNEW %i", dwBand);

		if (ret)
		{
			SetProp(ret, L"UIA_WindowVisibilityOverriden", (HANDLE)2);
			SetProp(ret, L"explorer7.WindowBand", (HANDLE)dwBand);
		}

		return ret;
	}
	else // Ittr: Preserve legacy codepath for win8.x and non-UWP users
	{
		DWORD p0 = (DWORD)_ReturnAddress();
		dwStyle = dwStyle | WS_EX_TOOLWINDOW;
		HWND ret = CreateWindowInBandOrig(dwExStyle, (LPWSTR)lpClassName, (PVOID)lpWindowName, (PVOID)dwStyle, (PVOID)x, (PVOID)y, (PVOID)nWidth, (PVOID)nHeight, hwndParent, hMenu, hInstance, lpParam, dwBand & 1);
		dbgprintf(L"%p: CreateWindowInBand %p %s %p %p %p %p %p %p %p %p %p %p %p = %p %p", p0, dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hwndParent, hMenu, hInstance, lpParam, dwBand, ret, GetLastError());
		SetProp(ret, L"explorer7.WindowBand", (HANDLE)dwBand);
		return ret;
	}
}

HWND WINAPI CreateWindowInBandExNew(DWORD exStyle, LPWSTR szClassName, PVOID p3, PVOID p4, PVOID p5, PVOID p6, PVOID p7, PVOID p8, PVOID p9, PVOID p10, PVOID p11, PVOID p12, DWORD p13, DWORD dwTypeFlags)
{
	DWORD p0 = (DWORD)_ReturnAddress();
	exStyle = exStyle | WS_EX_TOOLWINDOW;
	HWND ret = CreateWindowInBandExOrig(exStyle, szClassName, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 & 1, dwTypeFlags);

	// Ittr: We do this to eliminate the ghost window. The power of trans rights compelled me to fix this :3
	BOOL shouldCloak = true;
	WCHAR titleBuffer[MAX_PATH];
	GetClassName(ret, titleBuffer, sizeof(titleBuffer));
	WCHAR afwTitle[23] = L"ApplicationFrameWindow";
	if (strcmp((char*)titleBuffer, (char*)afwTitle) == 0)
		DwmSetWindowAttribute(ret, DWMWA_CLOAK, &shouldCloak, sizeof(shouldCloak));

	dbgprintf(L"%p: CreateWindowInBandEx %p %s %p %p %p %p %p %p %p %p %p %p %p = %p %p", p0, exStyle, szClassName, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, ret, GetLastError());
	dbgprintf(L"CreateWindowInBandExOrig %i", p13);

	SetProp(ret, L"UIA_WindowVisibilityOverriden", (HANDLE)2);
	SetProp(ret, L"explorer7.WindowBand", (HANDLE)p13);
	return ret;
}

BOOL WINAPI SetWindowBandNew(HWND hwnd, HWND hwndInsertAfter, DWORD flags)
{
	SetProp(hwnd, L"explorer7.WindowBand", (HANDLE)flags);
	dbgprintf(L"SetWindowBandNew %i", flags);
	return TRUE;
}

BOOL WINAPI ReturnZero()
{
	return TRUE;
}

BOOL WINAPI RegisterWindowHotkeyNew(HWND hwnd, int id, UINT mod, UINT vk)
{
	BOOL res = RegisterHotKeyApiOrg(hwnd, id, mod, vk);

	if (!res)
	{
		return TRUE;
	}

	return TRUE;
}

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
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

HMODULE GetCurrentModuleHandle() //use for internal resource calls... honestly i just wanted to show it could be done 
{
	HMODULE hMod = NULL;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle), &hMod);
	return hMod;
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

HRESULT WINAPI SHCoCreateInstanceNew(PCWSTR pszCLSID, const CLSID* pclsid, IUnknown* pUnkOuter, IID& riid, void** ppv)
{
	HRESULT res = SHCoCreateInstance(pszCLSID, pclsid, pUnkOuter, riid, ppv);
	if (res != S_OK && riid == GUID_88df9332_6adb_4604_8218_508673ef7f8a)
	{
		IShellURL10* shellurl10;
		res = SHCoCreateInstance(pszCLSID, pclsid, pUnkOuter, GUID_4f33718d_bae1_4f9b_96f2_d2a16e683346, (void**)&shellurl10);
		*ppv = new CShellURLWrapper(shellurl10);
	}
	return res;
}

void DisableWin11AltTab()
{
	if (g_osVersion.BuildNumber() >= 21996) // build check because this is unnecessary for windows 10
	{
		//Ittr: Why? Because it causes it to crash and its stupid
		char* immersiveBytes = "40 53 48 83 EC 20 83 79 ?? 02 74 17";

		//Load and patch DLL
		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };
		ChangeImportedPattern((char*)FindPattern((uintptr_t)LoadLibrary(L"twinui.pcshell.dll"), immersiveBytes), bytes, sizeof(bytes)); //byebye
	}
}

void FixWin11SearchIcon()
{
	// Ittr: An accidental change that actually works. Not complaining at all
	// Tested on 22000 and 26100
	// Not yet tested on Nickel (226xx)
	if (g_osVersion.BuildNumber() >= 21996) // build check because this is unnecessary for windows 10
	{
		char* searchBytes;

		if (g_osVersion.BuildNumber() >= 26100)
			searchBytes = "40 55 48 8B EC 48 83 EC 40"; // SHIsFileExplorerInTabletMode()
		else
			searchBytes = "48 89 5C 24 20 55 48 8B EC"; // SHIsFileExplorerInTabletMode()


		unsigned char bytes[] = { 0xB0, 0x00, 0xC3 };
		ChangeImportedPattern((char*)FindPattern((uintptr_t)LoadLibrary(L"ExplorerFrame.dll"), searchBytes), bytes, sizeof(bytes));
	}
}

void RemoveLoadAnimationDataMap()
{
	void* LoadAnimationDataMap = FindByString((uintptr_t)GetModuleHandle(L"uxtheme.dll"), L"AMAP");
	if (LoadAnimationDataMap)
	{
		LoadAnimationDataMap = (void*)GetFunctionStart((uintptr_t)LoadAnimationDataMap, (uintptr_t)GetModuleHandle(L"uxtheme.dll"));

		//byebye
		DWORD old;
		VirtualProtect(LoadAnimationDataMap, 1, PAGE_EXECUTE_READWRITE, &old);
		*reinterpret_cast<char*>(LoadAnimationDataMap) = 0xC3;
		VirtualProtect(LoadAnimationDataMap, 1, old, 0);
	}
}

void RemoveGetClassIdForShellTarget()
{
	void* GetClassIdForShellTarget = FindByString((uintptr_t)GetModuleHandle(L"uxtheme.dll"), L"Immersive");
	if (GetClassIdForShellTarget)
	{
		GetClassIdForShellTarget = (void*)GetFunctionStart((uintptr_t)GetClassIdForShellTarget, (uintptr_t)GetModuleHandle(L"uxtheme.dll"));

		//byebye
		DWORD old;
		VirtualProtect(GetClassIdForShellTarget, 1, PAGE_EXECUTE_READWRITE, &old);
		*reinterpret_cast<char*>(GetClassIdForShellTarget) = 0xC3;
		VirtualProtect(GetClassIdForShellTarget, 1, old, 0);
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

HRESULT(__fastcall* OnShellHookMessage)(void* a1, unsigned __int64 id, HWND a3);
HRESULT OnShellHookMessage_Hook(void* a1, unsigned __int64 id, HWND a3) //gets called when start menu is to be opened - a bit temperamental
{
	// key to note: at the moment, we can either do this for bugged start menu behaviour, or we can return S_OK and have no menu on the hotkey at all.
	// neither is ideal, but we can probably ship m2 like this and fix properly later

	if (CtaskBandPtr && id == 7)
		return CTaskBand_HandleShellHook(CtaskBandPtr,7,a3);

	return OnShellHookMessage(a1,id,a3);
}

// Ittr: Get rid of the immersive start menu and stop it appearing on TH1+ when UWP is on.
// This is very important and also extremely fragile.
// I'll also be honest - I haven't tested 1703 because who actually uses 1703
void DisableImmersiveStart()
{
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // because we don't want to run this thing if user isn't using UWP
	{
		char* ShowStartView; // XamlLauncher::ShowStartView
		unsigned char bytes[] = { 0xC3 }; // retn

		// load correct library - TH1 to RS1 use twinui, RS2 onwards use twinui.pcshell
		HMODULE twinui = (g_osVersion.BuildNumber() >= 15063) ? LoadLibrary(L"twinui.pcshell.dll") : LoadLibrary(L"twinui.dll");

		// so far there's only a few major revisions of this function as of 06-11-24
		// this may require further testing/advancement on windows 11 in co-ordination with partners
		if (g_osVersion.BuildNumber() >= 16299) // RS3 onwards
			ShowStartView = "48 89 5C 24 20 55 56 57 48 81 EC ?? 01 00 00 48 8B 05 ?? ?? ?? 00 48 33 C4 48 89 84 24 ?? 01 00 00 48 83 B9 ?? ?? 00 00 00";
		else if (g_osVersion.BuildNumber() >= 15063) // RS2
			ShowStartView = "48 89 5C 24 20 55 56 57 48 83 EC 30 48 83 B9 F8 00 00 00 00 41 8B E8";
		else if (g_osVersion.BuildNumber() >= 10074) // TH1 to RS1
			ShowStartView = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 83 B9 ?? 00 00 00 00";

		ChangeImportedPattern((char*)FindPattern((uintptr_t)twinui, ShowStartView), bytes, sizeof(bytes)); //byebye
	}

}

// Ittr: Get rid of the immersive search interface and prevent it appearing on TH1+ with ImmersiveShell enabled
// Otherwise, when invoked, takes up half the screen.
// Just use the Windows 7 start menu search - the functionality is much superior to this
void DisableImmersiveSearch()
{
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // because we don't want to run this thing if user isn't using UWP
	{
		char* CDEVSI; // CortanaDesktopExperienceView::ShowInternal
		// (preceded by CCortanaExperienceManager::ShowInternal in TH1-RS1)
		unsigned char bytes[] = { 0xC3 }; // retn

		// load correct library - TH1 to RS1 use twinui, RS2 onwards use twinui.pcshell (dll introduced in RS1 but not used widely)
		HMODULE twinui = (g_osVersion.BuildNumber() >= 15063) ? LoadLibrary(L"twinui.pcshell.dll") : LoadLibrary(L"twinui.dll");
		
		// seven different variants to account for as of 06-11-24
		if (g_osVersion.BuildNumber() >= 19041) // VB onwards
			CDEVSI = "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 20 57 48 83 EC 20 41 8B ?? 41 8B ?? 48 8B FA";
		else if (g_osVersion.BuildNumber() >= 18362) // 19H1 to 19H2
			CDEVSI = "40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 81 EC 80 00 00 00";
		else if (g_osVersion.BuildNumber() >= 17134) // RS4 to RS5
			CDEVSI = "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 20 57 48 83 EC 20 41 8B ?? 41 8B ?? 48 8B FA";
		else if (g_osVersion.BuildNumber() >= 16299) // RS3
			CDEVSI = "40 55 53 56 57 41 56 48 8D 6C 24 C9 48 81 EC 90 00 00 00";
		else if (g_osVersion.BuildNumber() >= 15063) // RS2
			CDEVSI = "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 20 41 8B D9 41 8B E8 48 8B F2";
		else if (g_osVersion.BuildNumber() >= 14393) // RS1
			CDEVSI = "40 55 53 56 57 41 56 48 8B EC 48 83 EC 70";
		else if (g_osVersion.BuildNumber() >= 10240) // TH1 to TH2
			CDEVSI = "48 8B C4 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC 90 00 00 00"; 

		// if user is using 19H1 or higher, search was reimplemented, which means we kill it twice
		if (g_osVersion.BuildNumber() >= 18362) 
		{
			// because once wasn't enough.

			char* SCFOS; // XamlLauncherState::ShowCortanaFromOpenStart
			// exists in RS5, but not used until 19H1
			// replaced by XamlLauncherState::ShowSearchFromOpenStart in W11 Nickel

			if (g_osVersion.BuildNumber() >= 22621) // W11 Nickel onwards
				SCFOS = "48 89 54 24 10 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC";
			else if (g_osVersion.BuildNumber() >= 19041) // VB onwards
				SCFOS = "48 89 54 24 10 55 53 56 57 41 56 41 57 48 8B EC 48 83 EC";
			else if (g_osVersion.BuildNumber() >= 18362) // 19H1 to 19H2
				SCFOS = "48 89 54 24 10 55 53 56 57 41 56 48 8B EC 48 83 EC 40 48 C7 45 E0 FE FF FF FF";

			ChangeImportedPattern((char*)FindPattern((uintptr_t)twinui, SCFOS), bytes, sizeof(bytes)); //byebye again
		}

		ChangeImportedPattern((char*)FindPattern((uintptr_t)twinui, CDEVSI), bytes, sizeof(bytes)); //byebye
	}
}

// Ittr: Get rid of the half-broken TaskView interface and prevent it appearing on TH1+ when UWP is on.
// This isn't detrimental to user experience to enable, but it completely breaks with the intended user experience.
// TaskView also causes crashing on earlier (pre-GE) Windows 11 which we can now avoid by disabling the remains of the feature.
// For some reason, Germanium and later already disable this. We're not complaining.
void DisableTaskView()
{
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // because we don't want to run this thing if user isn't using UWP
	{
		char* TaskViewHostShow; // XamlAllUpViewHost::Show 
		// (preceded by CAllUpViewHost::Show in TH1-RS1, replaced by TaskViewHost::Show in W11 Nickel)
		unsigned char bytes[] = { 0xC3 }; // retn

		// load correct library - TH1 to RS1 use twinui, RS2 onwards use twinui.pcshell (dll introduced in RS1 but not used widely)
		HMODULE twinui = (g_osVersion.BuildNumber() >= 15063) ? LoadLibrary(L"twinui.pcshell.dll") : LoadLibrary(L"twinui.dll");

		// this function is particularly annoying - the signature is different in some way for many versions of Windows 10/11
		// in some cases, it changes and reverts again in later versions
		// :/
		if (g_osVersion.BuildNumber() >= 22621) // W11 Nickel onwards
			TaskViewHostShow = "40 53 56 57 41 54 41 55 41 56 41 57 48 81 EC 30 03 00 00";
		else if (g_osVersion.BuildNumber() >= 21996) // W11 Cobalt
			TaskViewHostShow = "48 89 74 24 20 57 41 54 41 55 41 56 41 57 48 81 EC 20 03 00 00";
		else if (g_osVersion.BuildNumber() >= 19041) // VB
			TaskViewHostShow = "48 89 5C 24 20 56 57 41 54 41 55 41 57 48 81 EC";
		else if (g_osVersion.BuildNumber() >= 17763) // RS5 to 19H2
			TaskViewHostShow = "4C 8B DC 57 41 54 41 55 41 56 41 57 48 81 EC 40 03 00 00";
		else if (g_osVersion.BuildNumber() >= 15063) // RS2 to RS4
			TaskViewHostShow = "4C 8B DC ?? 41 54 41 55 41 56 41 57 48 83 EC";
		else if (g_osVersion.BuildNumber() >= 10586) // TH2 to RS1
			TaskViewHostShow = "48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? ?? ?? ?? ?? 48 81 EC 50 02 00 00";
		else if (g_osVersion.BuildNumber() >= 10074) // TH1
			TaskViewHostShow = "48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 ?? ?? ?? ?? ?? ?? ?? ?? 48 81 EC E0 01 00 00";

		ChangeImportedPattern((char*)FindPattern((uintptr_t)twinui, TaskViewHostShow), bytes, sizeof(bytes)); //byebye
	}
}

// Ittr: New method for removing immersive menus. Better inter-operability between Windows versions. Used alongside existing method.
BOOL SystemParametersInfoWNEW(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni)
{
	if (uiAction == SPI_GETSCREENREADER)
	{
		*(BOOL*)pvParam = TRUE;
		return TRUE;
	}

	return SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);
}

//Ittr: Goodbye immersive context menus and good riddance. For Win10 TH1+. In future consider build check to limit to 10240+. 
//Also to be noted that Windows 11 makes further changes here that we'll need to account for in future if we do officially support it.
void ShowWin32Menus()
{
	if (g_osVersion.BuildNumber() >= 10074) // if user is using TH1 or later
	{
		char* CAODTM_SH32; // ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu
		char* CAODTM_EF; // same function, in ExplorerFrame.dll
		char unsigned bytes[] = { 0xC3 }; // retn

		if (g_osVersion.BuildNumber() >= 26100) // W11 Germanium onwards
		{
			CAODTM_SH32 = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 33 DB 48 8B F2 33 FF 48 8B E9";
			CAODTM_EF = CAODTM_SH32;
		}
		else if (g_osVersion.BuildNumber() >= 21996) // W11 Cobalt to W11 Nickel
		{
			// This is somewhat flawed on Cobalt, but it will have to do
			CAODTM_SH32 = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B E9 33 FF 33 D2";
			CAODTM_EF = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B E9 33 FF 33 D2 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? C7 44 24 20 50 00 00 00";
		}
		else if (g_osVersion.BuildNumber() >= 10074) // TH1 to VB
		{
			CAODTM_SH32 = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 70 48 8B F2 48 8B";
			CAODTM_EF = CAODTM_SH32;
		}

		ChangeImportedPattern((char*)FindPattern((uintptr_t)GetModuleHandle(L"shell32.dll"), CAODTM_SH32), bytes, sizeof(bytes)); // shell32.dll
		ChangeImportedPattern((char*)FindPattern((uintptr_t)LoadLibrary(L"ExplorerFrame.dll"), CAODTM_EF), bytes, sizeof(bytes)); // ExplorerFrame.dll

		// Ensure as much as we can that it's gone, if the above isn't enough (Win11 Cobalt, I'm looking at you...)
		if (g_osVersion.BuildNumber() >= 10074)
		{
			// Only applied to shell32, as application to ExplorerFrame breaks the program list hover behaviour.
			ChangeImportedAddress(GetModuleHandle(L"shell32.dll"), "user32.dll", SystemParametersInfoW, SystemParametersInfoWNEW);
		}
	}
}

void HookShell32();
void HookAPIs()
{
	// Before doing anything else, initialize the registry switch for immersive shell as this determines what hooks and changes are needed
	DWORD dwEnableUWP = 0;
	g_registry.QueryValue(L"EnableImmersive", (LPBYTE)&dwEnableUWP, sizeof(DWORD));
	g_bEnableImmersiveShellStack = dwEnableUWP;

	// Windows 8-esque mode. Lol
	DWORD dwRPEnabled = 0;
	g_registry.QueryValue(L"RPEnabled", (LPBYTE)&dwRPEnabled, sizeof(DWORD));
	g_bRPEnabled = dwRPEnabled;

	// Change and fix core desktop components
	hEvent_DesktopVisible = CreateEvent(NULL, TRUE, FALSE, L"ShellDesktopVisibleEvent");
	SHCreateDesktopOrig = (SHCreateDesktopAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"), (LPSTR)200);
	ChangeImportedAddress(GetModuleHandle(NULL), "shell32.dll", SHCreateDesktopOrig, SHCreateDesktopNEW);
	SHDesktopMessageLoop = (SHCreateDesktopAPI)GetProcAddress(GetModuleHandle(L"shell32.dll"), (LPSTR)201);
	ChangeImportedAddress(GetModuleHandle(NULL), "shell32.dll", SHDesktopMessageLoop, SHDesktopMessageLoopNEW);

	// Fixes the "search by extension" feature in the start menu
	ChangeImportedAddress(GetModuleHandle(NULL), "shell32.dll", GetProcAddress(GetModuleHandle(L"shell32.dll"), "SHCoCreateInstance"), SHCoCreateInstanceNew);

	// Change appid
	ChangeImportedAddress(GetModuleHandle(NULL), "kernel32.dll", SetErrorMode, SetErrorModeNEW);

	// Disable DWM composition as quickly as we can (if registry key set)
	ChangeImportedAddress(GetModuleHandle(NULL), "uxtheme.dll", IsCompositionActive, IsCompositionActiveNEW);
	ChangeImportedAddress(GetModuleHandle(NULL), "dwmapi.dll", DwmIsCompositionEnabled, DwmIsCompositionEnabledNEW);
	//ChangeImportedAddress(GetModuleHandle(NULL), "user32.dll", RegisterWindowMessageW, RegisterWindowMessageWNEW);

	// 1. Remove Windows 8+ animation msstyle classes so that legacy msstyles from Vista onwards are compatible with our theming system
	// 2. Remove Windows 8+ immersive shell msstyle classes so that legacy msstyles from Vista onwards are compatible with our theming system
	RemoveLoadAnimationDataMap();
	RemoveGetClassIdForShellTarget();

	// Initialize the theme manager and declare the types for the UXTheme apis we're hooking
	ThemeManagerInitialize();
	fOpenThemeData = decltype(fOpenThemeData)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeData"));
	fOpenThemeDataForDpi = decltype(fOpenThemeDataForDpi)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeDataForDpi"));
	fOpenThemeDataEx = decltype(fOpenThemeDataEx)(GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "OpenThemeDataEx"));

	// ???
	ModifyDesktopHwnd();

	// We initialize the MinHook system here
	MH_Initialize();

	//CImmersiveWindowMessageService__RequestHotkeys = (decltype(CImmersiveWindowMessageService__RequestHotkeys))FindPattern((uintptr_t)LoadLibrary(L"twinui.dll"), "4C 8B DC 4D 89 43 ?? 57 41 54 41 55 41 56 41 57 48 83 EC");
	//MH_CreateHook(static_cast<LPVOID>(CImmersiveWindowMessageService__RequestHotkeys), CImmersiveWindowMessageService__RequestHotkeys_Hook, reinterpret_cast<LPVOID*>(&CImmersiveWindowMessageService__RequestHotkeys));

	fRegisterWindowMessageW = (decltype(fRegisterWindowMessageW))GetProcAddress(LoadLibraryW(L"user32.dll"),"RegisterWindowMessageW");

	// disabled - <1607 doesnt like atm + unfinished. sorry! uncomment if you're testing
	CTaskBand_HandleShellHook = (decltype(CTaskBand_HandleShellHook))FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 08 55 56 57 41 54 41 55 48 83 EC ?? 83 FA 07");
	OnShellHookMessage = (decltype(OnShellHookMessage))FindPattern((uintptr_t)LoadLibraryW(L"twinui.pcshell.dll"), "40 53 48 83 EC 20 48 8B D9 48 8B 89 ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 48 8B 40 ?? FF 15 ?? ?? ?? ?? 84 C0 0F 85 ?? ?? ?? ?? 38 83");

	// Hook UXTheme-related calls for the purpose of our inactive theme system.
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeData), OpenThemeData_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeData));
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeDataForDpi), OpenThemeDataForDpi_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeDataForDpi));
	MH_CreateHook(static_cast<LPVOID>(fOpenThemeDataEx), OpenThemeDataEx_Hook, reinterpret_cast<LPVOID*>(&fOpenThemeDataEx));
	// disabled - <1607 doesnt like atm + unfinished. sorry! uncomment if you're testing
	MH_CreateHook(static_cast<LPVOID>(OnShellHookMessage), OnShellHookMessage_Hook, reinterpret_cast<LPVOID*>(&OnShellHookMessage));

	// Hook and update definitions of what windows should be added to the tray - largely for UWP purposes, but essentially zero-cost so included on both immersive on and off modes.
	void* _ShouldAddWindowToTray = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8B F9 33 DB");
	void* _IsWindowNotDesktopOrTray = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 ?? 57 48 83 EC ?? 48 8B F9 33 DB FF 15 ?? ?? ?? ?? 3B C3 74 ?? 48 3B 3D");
	MH_CreateHook(static_cast<LPVOID>(_ShouldAddWindowToTray), ShouldAddWindowToTray, reinterpret_cast<LPVOID*>(&_ShouldAddWindowToTray));
	MH_CreateHook(static_cast<LPVOID>(_IsWindowNotDesktopOrTray), IsWindowNotDesktopOrTray, reinterpret_cast<LPVOID*>(&_IsWindowNotDesktopOrTray));
	
	// thumbnail fix
	if (g_osVersion.BuildNumber() >= 10074) // we don't apply to 8.1 as only pseudo-aero is supported there
	{
		void* _thumbnailrender = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 20 44 89 40 18 57 41 54 41 55 41 56 41 57 48 81 EC 90 00 00 00 48 8B F9");
		MH_CreateHook(static_cast<LPVOID>(_thumbnailrender), RenderThumbnail, reinterpret_cast<LPVOID*>(&renderThumbnail_orig));
	}

	// 1. Todo in future *after* feature-set is complete: see how many of these hooks can be ChangeImportedAddress instead of MH_CreateHook (perf optimisation)
	// 2. Code stack used exclusively for UWP mode, hence the conditional statement.
	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // Ittr: Run these hooks only if the user A) is on Windows 10 and B) has UWP enabled
	{
		// 1. This will *need* serious optimization in the near future as it singlehandedly delays program enumeration and startup by several seconds
		// 2. Prepare the taskbar and thumbnails to handle UWP icons. Further work needed for jumplists and to prevent wrongful classification as "Application Frame Host" in the first place.
		void* _ctaskbandadd = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "FF F3 55 56 57 41 54 41 55 41 56 41 57 48 81 EC F8 06 00 00");
		void* _cthumbnailUpdate = (void*)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 48 8B 81 B0 00 00 00");
		SetIconThumb = (setIconThumb_t)FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 63 D8 4C 8B 81 B0 00 00 00");

		MH_CreateHook(static_cast<LPVOID>(_ctaskbandadd), SetWindowIcon, reinterpret_cast<LPVOID*>(&SetIcon));
		MH_CreateHook(static_cast<LPVOID>(_cthumbnailUpdate), UpdateItemIcon, reinterpret_cast<LPVOID*>(&UpdateItem));

		// The rest of this code block is dedicated to ensuring UWP actually runs in the first place
		CreateWindowInBandOrig = decltype(CreateWindowInBandOrig)(GetProcAddress(GetModuleHandle(L"user32.dll"), "CreateWindowInBand"));
		CreateWindowInBandExOrig = decltype(CreateWindowInBandExOrig)(GetProcAddress(GetModuleHandle(L"user32.dll"), "CreateWindowInBandEx"));
		SetWindowBandApiOrg = decltype(SetWindowBandApiOrg)(GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowBand"));
		RegisterHotKeyApiOrg = decltype(RegisterHotKeyApiOrg)(GetProcAddress(GetModuleHandle(L"user32.dll"), "RegisterHotKey"));

		MH_CreateHook(static_cast<LPVOID>(CreateWindowInBandOrig), CreateWindowInBandNew, reinterpret_cast<LPVOID*>(&CreateWindowInBandOrig));
		MH_CreateHook(static_cast<LPVOID>(CreateWindowInBandExOrig), CreateWindowInBandExNew, reinterpret_cast<LPVOID*>(&CreateWindowInBandExOrig));
		MH_CreateHook(static_cast<LPVOID>(SetWindowBandApiOrg), SetWindowBandNew, reinterpret_cast<LPVOID*>(&SetWindowBandApiOrg));
		//MH_CreateHook(static_cast<LPVOID>(RegisterHotKeyApiOrg), RegisterWindowHotkeyNew, reinterpret_cast<LPVOID*>(&RegisterHotKeyApiOrg));

		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2581), ReturnZero, NULL); // GetWindowTrackInfoAsync
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2563), ReturnZero, NULL); // ClearForeground
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2628), ReturnZero, NULL); // CreateWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2629), ReturnZero, NULL); // DeleteWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2631), ReturnZero, NULL); // EnableWindowGroupPolicy
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2627), ReturnZero, NULL); // SetBridgeWindowChild
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2511), ReturnZero, NULL); // SetFallbackForeground
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2566), ReturnZero, NULL); // SetWindowArrangement
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2632), ReturnZero, NULL); // SetWindowGroup
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2579), ReturnZero, NULL); // SetWindowShowState
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2585), ReturnZero, NULL); // UpdateWindowTrackingInfo
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2514), ReturnZero, NULL); // RegisterEdgy
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2542), ReturnZero, NULL); // RegisterShellPTPListener
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2537), ReturnZero, NULL); // SendEventMessage
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2513), ReturnZero, NULL); // SetActiveProcessForMonitor
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2564), ReturnZero, NULL); // RegisterWindowArrangementCallout
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), (LPCSTR)2567), ReturnZero, NULL); // EnableShellWindowManagementBehavior
		MH_CreateHook(GetProcAddress(GetModuleHandle(L"user32.dll"), "AllowSetForegroundWindow"), ReturnZero, NULL);
	}

	// If we are on Windows 10 or higher, query the original program list pattern and create our hook to fix the visual issues
	if (g_osVersion.BuildNumber() >= 10240)
	{
		CNSCHost_FillNSCOg = (decltype(CNSCHost_FillNSCOg))FindPattern((uintptr_t)GetModuleHandle(0), "48 89 5C 24 18 57 48 83 EC 30 33 DB 48 8B F9 39 99 CC 00 00 00");
		if (CNSCHost_FillNSCOg)
			MH_CreateHook(static_cast<LPVOID>(CNSCHost_FillNSCOg), CNSCHost_FillNSC, reinterpret_cast<LPVOID*>(&CNSCHost_FillNSCOg)); //this hook is in nsctree.h now
	}

	// Prevent theme overrides applying to file explorer *VERY IMPORTANT*
	HookTrayThread();

	// Adapt colorization api
	DwmGetColorizationParametersOrig = (SHPtrParamAPI)GetProcAddress(GetModuleHandle(L"dwmapi.dll"), (LPSTR)127);
	DwmpActivateLivePreview = (decltype(DwmpActivateLivePreview))GetProcAddress(GetModuleHandle(L"dwmapi.dll"), (LPSTR)113);
	ChangeImportedAddress(GetModuleHandle(NULL), "dwmapi.dll", DwmpActivateLivePreview, DwmpActivateLivePreviewNEW);
	ChangeImportedAddress(GetModuleHandle(NULL), "dwmapi.dll", DwmGetColorizationParametersOrig, DwmGetColorizationParametersNEW);

	// Add DWM colorization attributes to taskbar and start menu (depending on whether mode is 0 aka legacy or 1-3 aka new options) (how this renders is theme-dependent).
	// Currently not working for taskbar thumbnails from 1809 onwards...
	SetWindowCompositionAttribute = (SetWindowCompositionAttributeAPI)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");
	DwmpUpdateAccentBlurRect = (DwmpUpdateAccentBlurRect_t)GetProcAddress(GetModuleHandle(L"dwmapi.dll"), (LPSTR)159);
	ChangeImportedAddress(GetModuleHandle(NULL), "user32.dll", SetWindowCompositionAttribute, SetWindowCompositionAttributeNEW);
	ChangeImportedAddress(GetModuleHandle(NULL), "dwmapi.dll", DwmEnableBlurBehindWindow, DwmEnableBlurBehindWindowNEW);
	ChangeImportedAddress(GetModuleHandle(NULL), "user32.dll", SetWindowRgn, SetWindowRgnNEW);

	// Load functions needed for task enumeration hook
	HMODULE user32 = LoadLibrary(L"user32.dll");
	IsShellFrameWindow = (IsShellWindow_t)GetProcAddress(user32, (LPCSTR)2573);
	GhostWindowFromHungWindow = (GhostWindowFromHungWindow_t)GetProcAddress(user32, "GhostWindowFromHungWindow");
	ChangeImportedAddress(GetModuleHandle(NULL), "user32.dll", IsWindowVisible, IsWindowVisibleNEW); // perform the actual hook

	// Change show desktop button for Windows 8-based themes
	ChangeImportedAddress(GetModuleHandle(NULL), "uxtheme.dll", SetWindowTheme, SetWindowThemeNEW);

	// Update overflow positioning to account for if the user is using TH1 or higher
	if (g_osVersion.BuildNumber() >= 10074)
		ChangeImportedAddress(GetModuleHandle(NULL), "user32.dll", GetProcAddress(GetModuleHandle(L"user32.dll"), (LPSTR)"CalculatePopupWindowPosition"), CalculatePopupWindowPositionNEW);
	
	// 1. shell32.dll - hack created startmenupin instance
	// 2. shell32.dll - patch delayload stuff
	StartMenuPin_PatchShell32();
	HookShell32();

	// Assorted fixes and changes
	DisableImmersiveStart(); // Remove Windows 10+ immersive start menu for UWP mode (doesn't fix hotkeys yet)
	DisableImmersiveSearch(); // Remove Windows 10+ immersive search menu for UWP mode
	DisableTaskView(); // Remove Windows 10+ virtual desktops functionality for UWP mode
	ShowWin32Menus(); // Remove immersive menus so taskbar behaves properly
	FixAuthUI(); // Responsible for fixing CLogoffOptions
	DisableWin11AltTab(); // Disable XAML UI because it crashes (Win+Tab will still need to separately be accounted for on Cobalt and possibly Nickel. M3?)
	FixWin11SearchIcon(); // Prevents search icon from being mangled by a buggy tablet mode implementation (cheers Microsoft)

	// Query registry for disable composition value
	DWORD dwDisableComposition = 0;
	g_registry.QueryValue(L"DisableComposition", (LPBYTE)&dwDisableComposition, sizeof(DWORD));
	g_bDisableComposition = (dwDisableComposition != 0);

	// Query registry for forced classic theme value
	DWORD dwClassicTheme = 0;
	g_registry.QueryValue(L"ClassicTheme", (LPBYTE)&dwClassicTheme, sizeof(DWORD));
	if (dwClassicTheme != 0)
	{
		dbgprintf(L"setting classic theme");
		g_bDisableComposition = true; // classic theme never had comp, duh
		g_bClassicTheme = true;
		SetThemeAppProperties(NULL); // method needs future improvement here...
	}

	// Query registry for colorization option selected by the user
	DWORD dwColorizationOptions = 1; // default to pseudo-aero from now on - mode 0 remains present for milestone 2
	g_registry.QueryValue(L"ColorizationOptions", (LPBYTE)&dwColorizationOptions, sizeof(DWORD));
	if (dwColorizationOptions != 0 && dwColorizationOptions < 5) // if outside the boundaries, defaults back to 0
	{
		if (dwColorizationOptions == 2 && g_osVersion.BuildNumber() >= 22621) // BlurBehind is broken from Nickel onwards, so we enforce acrylic instead as an alternative blur effect...
			g_bColorizationOptions = 3;
		else if (dwColorizationOptions == 3 && g_osVersion.BuildNumber() < 17134) // Acrylic is not added to Win32 api until RS4, so fall back to pseudo-aero for wider OS consistency...
			g_bColorizationOptions = 1;
		else if (dwColorizationOptions >= 2 && g_osVersion.BuildNumber() < 10074) // BlurBehind, Acrylic, SolidColor unsupported on 8.x
			g_bColorizationOptions = 1;
		else // e.g. you're using a supported mode on your OS
			g_bColorizationOptions = dwColorizationOptions;

	}

	// Query registry for alpha override
	DWORD dwOverrideAlpha = 0;
	g_registry.QueryValue(L"OverrideAlpha", (LPBYTE)&dwOverrideAlpha, sizeof(DWORD));
	g_bOverrideAlpha = dwOverrideAlpha;

	// Query registry for custom alpha value, falls back to default Win7 0x6B if no value is set
	DWORD dwAlphaValue = 0x6B;
	g_registry.QueryValue(L"AlphaValue", (LPBYTE)&dwAlphaValue, sizeof(DWORD));
	g_bAlphaValue = dwAlphaValue;

	// Query registry for which acrylic style to use (if user is using colorization option 3)
	DWORD dwAcrylicAlt = 0;
	g_registry.QueryValue(L"AcrylicColorization", (LPBYTE)&dwAcrylicAlt, sizeof(DWORD));
	g_bAcrylicAlt = dwAcrylicAlt;

	//fix classic start menu icon (pls fix)
	/*HMODULE winbrand = LoadLibrary(L"winbrand.dll");
	BrandingLoadImage = (BrandingLoadImage_t)GetProcAddress(winbrand, "BrandingLoadImage");
	if (BrandingLoadImage)
		ChangeImportedAddress(GetModuleHandle(NULL),"winbrand.dll",BrandingLoadImage,BrandingLoadImageNEW);*/

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

	if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074)
		CreateWindowInBandExOrig = (CreateWindowInBandExAPI)GetProcAddress(hUser32, "CreateWindowInBand");

	GetWindowBandOrig = (GetWindowBandAPI)GetProcAddress(hUser32, "GetWindowBand");
	ChangeImportedAddress(immersiveui, "user32.dll", CreateWindowInBandOrig, CreateWindowInBandNew);
	ChangeImportedAddress(immersiveui, "user32.dll", GetWindowBandOrig, GetWindowBandNew);
	ChangeImportedAddress(immersiveui, "user32.dll", GetUserObjectInformation, GetUserObjectInformationNew);
	ChangeImportedAddress(immersiveui, "user32.dll", SetTimer, SetTimer_WUI);

	if (!g_bEnableImmersiveShellStack || g_osVersion.BuildNumber() < 10074) // Ittr: If user *either* has UWP disabled, or they are NOT on Windows 10, run legacy window band code
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

// Generic crash error
void CrashError()
{
	WCHAR errorText[71] = L"An unexpected error occurred and explorer7 needs to quit. We're sorry!"; // Funny brick game message go haha
	WCHAR errorTitle[16] = L"explorer7 Crash";

	MessageBoxW(NULL, errorText, errorTitle, MB_ICONERROR); // the actual error box lol
}

// Create all programs shellfolder on 1607+ where it doesn't already exist
void CreateShellFolder()
{
	//addendum: using the regular HKLM location is not viable for non-administrator users so we store in HKCU, which causes it to turn up in HKEY_USERS somewhere. 
	//this shouldn't work, but it does :P
	if (g_osVersion.BuildNumber() >= 14393) // Ittr: byebye shellfolder.reg
	{
		DWORD value = 0; // initialise in memory
		DWORD attrVal = 0x28100000; // doesn't work when reduced to a single string, annoying but atleast we can use it here
		RegGetDWORD(HKEY_CURRENT_USER, sz_ShellFolder3, L"Attributes", &value); // output the data from attributes key...

		if (value != attrVal) // basically if the attribute value doesn't exist or is the wrong value...
		{
			// we create all the relevant values. issue solved for new users - program list works out of the box now
			RegSetSZ(HKEY_CURRENT_USER, sz_ShellFolder, NULL, (DWORD*)L"Programs Folder and Fast Items"); // create clsid name
			RegSetExpandSZ(HKEY_CURRENT_USER, sz_ShellFolder2, NULL, (DWORD*)L"%SystemRoot%\\system32\\shell32.dll"); // point it to shell32
			RegSetSZ(HKEY_CURRENT_USER, sz_ShellFolder2, L"ThreadingModel", (DWORD*)L"Apartment"); // regular threading model criteria...
			RegSetDWORD(HKEY_CURRENT_USER, sz_ShellFolder3, L"Attributes", &attrVal); // apply folder attributes, arguably the most important part
		}
	}
}

// Compatibility warning for Windows 11 (Milestone 2)
void FirstRunCompatibilityWarning()
{
	if (g_osVersion.BuildNumber() >= 21996 || g_osVersion.BuildNumber() == 20348) // temporary one-off M2 warning for win11 users, permanent for iron users
	{
		DWORD value = 0;
		RegGetDWORD(HKEY_CURRENT_USER, sz_SettingsKey, L"FirstRunVersionCheck", &value);
		if (value != 1)
		{
			MessageBoxW(NULL, L"This build of Windows is not currently supported.\n\nYou may encounter usability issues.", L"explorer7", MB_ICONEXCLAMATION);
			DWORD newValue = 1;
			RegSetDWORD(HKEY_CURRENT_USER, sz_SettingsKey, L"FirstRunVersionCheck", &newValue);
		}
	}
}

// One-off warning for pre-release version
void FirstRunPrereleaseWarning()
{
	DWORD value = 0;
	RegGetDWORD(HKEY_CURRENT_USER, sz_SettingsKey, L"FirstRunPrereleaseCheck", &value);
	if (value != 1)
	{
#ifdef PRERELEASE_COPY
		MessageBoxW(NULL, L"Evaluation copy.\nFor testing purposes only.", L"explorer7", MB_ICONEXCLAMATION);
		DWORD newValue = 1;
		RegSetDWORD(HKEY_CURRENT_USER, sz_SettingsKey, L"FirstRunPrereleaseCheck", &newValue);
#endif
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

		CreateShellFolder(); // Fix shell folder for 1607+...
		EnsureWindowColorization(); // Correct colorization enablement setting for Win10/11
		FirstRunCompatibilityWarning(); // Warn users on Windows 11 (for milestone 2) and Server 2022 of potential problems
		FirstRunPrereleaseWarning(); // Warn users if this is a pre-release build that this is the case on first run ONLY
		ThemeHandlesInit(); // Basically start the inactive theme management process

		dbgprintf(L"Dll Attach\n");
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

		if (g_bEnableImmersiveShellStack && g_osVersion.BuildNumber() >= 10074) // Ittr: Only create TWinUI UWP mode here if we are going to use it
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
			id = IID_IFlexibleTaskbarPinnedList;
		else if (build >= 17763)
			id = IID_IPinnedList3;

		result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, id, ppv);
		*ppv = new CPinnedListWrapper((IUnknown*)*ppv, build);
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
		if (g_osVersion.BuildNumber() < 10074) // Ittr: gate fakeimmersive to 8.1 due to functional issues (e.g. hanging) with 10 - restoring this on 10 is now seemingly unnecessary
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
		if (g_osVersion.BuildNumber() < 10240) // Ittr: gate fakeimmersive to 8.1 due to functional issues (e.g. hanging) with 10
		{
			UnregisterFakeImmersive();
			UnregisterProjection();
		}
	}
	return CoRevokeClassObject(dwRegister);
}