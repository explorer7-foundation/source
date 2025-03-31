#pragma once
#include "common.h"

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

typedef VOID(WINAPI* CPniMainDlg_ShowFlyout_t)(HWND* hwnd, char a2);
static CPniMainDlg_ShowFlyout_t CPniMainDlg_ShowFlyout;

//typedef VOID(WINAPI* CTaskbandPin_CreateInstance_t)(PVOID);
//static CTaskbandPin_CreateInstance_t CTaskbandPin_CreateInstance;

class CTaskbandPin_W32PTP;
typedef HRESULT(*CTaskbandPin_CreateInstance_t)(CTaskbandPin_W32PTP**);
static CTaskbandPin_CreateInstance_t CTaskbandPin_CreateInstance;

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
	IMCLR_SystemAccentDark2 = 0x6,
	IMCLR_HardwareGutterRest = 0x50 // Replaces IMCLR_SystemAccent, as it is more accurate to DWM colorization

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

__int64(__fastcall* DwmpActivateLivePreview)(int a1, __int64 a2, __int64 a3, int a4, void* a5);

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

typedef BOOL(*IsShellManagedWindow_t)(HWND hwnd); // 2574

HTHEME(__stdcall* fOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
HTHEME(__stdcall* fOpenThemeDataForDpi)(HWND hwnd, LPCWSTR pszClassList, UINT dpi);
HTHEME(__stdcall* fOpenThemeDataEx)(HWND hwnd, LPCWSTR pszClassList, DWORD dwFlags);

typedef int(*IsThemeClassDefined_t)(HTHEME hTheme, LPCWSTR pszAppName, LPCWSTR pszClassId, int fAllowInheritance);
IsThemeClassDefined_t IsThemeClassDefined;

LPTHREAD_START_ROUTINE CTray__SyncThreadProc_orig = nullptr;

// prevent windows 11 hotkey registration
typedef BOOL(WINAPI* ShellRegisterHotKey_t)(HWND, int, UINT, UINT, HWND);
static ShellRegisterHotKey_t ShellRegisterHotKey;

