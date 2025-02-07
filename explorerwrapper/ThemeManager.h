#pragma once
#include "common.h"

struct UXTHEMEFILE
{
	char header[7]; // must be "thmfile"
	LPVOID sharableSectionView;
	HANDLE hSharableSection;
	LPVOID nsSectionView;
	HANDLE hNsSection;
	char end[3]; // must be "end"

};

extern HRESULT(WINAPI *GetThemeDefaults)(
	LPCWSTR pszThemeFileName,
	LPWSTR  pszColorName,
	DWORD   dwColorNameLen,
	LPWSTR  pszSizeName,
	DWORD   dwSizeNameLen
);

extern HRESULT(WINAPI *LoaderLoadTheme)(
	HANDLE      hThemeFile,
	HINSTANCE   hThemeLibrary,
	LPCWSTR     pszThemeFileName,
	LPCWSTR     pszColorParam,
	LPCWSTR     pszSizeParam,
	OUT HANDLE *hSharableSection,
	LPWSTR      pszSharableSectionName,
	int         cchSharableSectionName,
	OUT HANDLE *hNonsharableSection,
	LPWSTR      pszNonsharableSectionName,
	int         cchNonsharableSectionName,
	PVOID       pfnCustomLoadHandler,
	OUT HANDLE *hReuseSection,
	int         a,
	int         b,
	BOOL        fEmulateGlobal
);

typedef HRESULT(WINAPI *LoaderLoadTheme_t_win11)(
	HANDLE      hThemeFile,
	HINSTANCE   hThemeLibrary,
	LPCWSTR     pszThemeFileName,
	LPCWSTR     pszColorParam,
	LPCWSTR     pszSizeParam,
	OUT HANDLE *hSharableSection,
	LPWSTR      pszSharableSectionName,
	int         cchSharableSectionName,
	OUT HANDLE *hNonsharableSection,
	LPWSTR      pszNonsharableSectionName,
	int         cchNonsharableSectionName,
	PVOID       pfnCustomLoadHandler,
	OUT HANDLE *hReuseSection,
	int         a,
	int         b
);

extern HTHEME(WINAPI *OpenThemeDataFromFile)(
	UXTHEMEFILE *lpThemeFile,
	HWND         hWnd,
	LPCWSTR      pszClassList,
	DWORD        dwFlags
);

extern UXTHEMEFILE *g_loadedTheme;

void ThemeManagerInitialize();
HRESULT LoadThemeFile(wchar_t* Path);