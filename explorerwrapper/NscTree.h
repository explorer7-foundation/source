#pragma once
#include "common.h"

DEFINE_GUID(CLSID_PersonalStartMenu, 0x3F6953F0, 0x5359, 0x47FC, 0x0BD, 0x99, 0x9F, 0x2C, 0x0B9, 0x5A, 0x62, 0x0FD);

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
INameSpaceTreeControlValuesPrivate : IUnknown
{
public:
	virtual void stub() = 0;
	virtual void SetIndentValue(int indent) = 0;
};

UINT (__fastcall*fGetDpiForWindow)(HWND hwnd);
DPI_AWARENESS_CONTEXT (__fastcall*fGetWindowDpiAwarenessContext)(HWND hwnd);
BOOL (__fastcall*fAreDpiAwarenessContextsEqual)(DPI_AWARENESS_CONTEXT A, DPI_AWARENESS_CONTEXT B);

static void __fastcall SHComputeDPI(HWND a1, int* a2, int* a3)
{
	DPI_AWARENESS_CONTEXT v7; // rax
	HDC DC; // rax
	HDC v9; // rdi
	int DeviceCaps; // ebx
	int v11; // esi

	if (a1 && (v7 = fGetWindowDpiAwarenessContext(a1), fAreDpiAwarenessContextsEqual(v7, (DPI_AWARENESS_CONTEXT)-4LL)))
	{
		DeviceCaps = fGetDpiForWindow(a1);
		v11 = DeviceCaps;
	}
	else
	{
		DC = GetDC(0LL);
		v9 = DC;
		if (DC)
		{
			DeviceCaps = GetDeviceCaps(DC, 88);
			v11 = GetDeviceCaps(v9, 90);
			ReleaseDC(0LL, v9);
		}
		else
		{
			DeviceCaps = 96;
			v11 = 96;
		}
	}
	if (a2)
		*a2 = DeviceCaps;
	if (a3)
		*a3 = v11;
}

//custom versions of the functions because ppl use patched dlls and aerexplorer messes this up
static void __fastcall CNscTree_ScaleAndSetIndent(__int64 a1)
{
	int v1; // ebx
	int v3; // eax
	int nNumerator; // [rsp+30h] [rbp+8h] BYREF
	int v6; // [rsp+38h] [rbp+10h] BYREF
	int extraOffset = 0;

	if (g_osVersion.BuildNumber() >= 26100 || (g_osVersion.BuildNumber() >= 22621 && g_osVersion.BuildRevision() >= 4602)) // Ittr: Handle windows 11 offset difference. Inefficient but simplified code wasnt working
		extraOffset = 16;
	else if (g_osVersion.BuildNumber() >= 21996)
		extraOffset = 8;

	v1 = *(DWORD*)(a1 + 0x1D0 + extraOffset);
	SHComputeDPI(*(HWND*)(a1 + 0x188 + extraOffset), &v6, &nNumerator);
	v3 = MulDiv(v1, nNumerator, 96);
	SendMessageW(*(HWND*)(a1 + 0x178 + extraOffset), 0x1107u, v3, 0LL);
}

static void __fastcall CNscTree_SetIndentValue(__int64 a1, int a2)
{
	int extraOffset = 0;

	if (g_osVersion.BuildNumber() >= 26100 || (g_osVersion.BuildNumber() >= 22621 && g_osVersion.BuildRevision() >= 4602)) // Ittr: Handle windows 11 offset difference. Inefficient but simplified code wasnt working
		extraOffset = 16;
	else if (g_osVersion.BuildNumber() >= 21996)
		extraOffset = 8;

	*(DWORD*)(a1 + 0xA0 + extraOffset) = a2;
	CNscTree_ScaleAndSetIndent(a1 - 304);
}

static void __fastcall CNscTree_ScaleAndSetRowHeight(__int64 a1)
{
	int v1; // ebp
	HWND v2; // rdi
	DPI_AWARENESS_CONTEXT v5; // rax
	HDC DC; // rax
	HDC v7; // rbx
	int DeviceCaps; // edi
	int v9; // eax
	int extraOffset = 0;

	if (g_osVersion.BuildNumber() >= 26100 || (g_osVersion.BuildNumber() >= 22621 && g_osVersion.BuildRevision() >= 4602)) // Ittr: Handle windows 11 offset difference. Inefficient but simplified code wasnt working
		extraOffset = 16;
	else if (g_osVersion.BuildNumber() >= 21996)
		extraOffset = 8;

	v1 = *(DWORD*)(a1 + 0x1C8 + extraOffset);
	v2 = *(HWND*)(a1 + 0x188 + extraOffset);
	if (v2 && (v5 = fGetWindowDpiAwarenessContext(v2), fAreDpiAwarenessContextsEqual(v5, (DPI_AWARENESS_CONTEXT)-4LL)))
	{
		DeviceCaps = fGetDpiForWindow(v2);
	}
	else
	{
		DC = GetDC(0LL);
		v7 = DC;
		if (DC)
		{
			GetDeviceCaps(DC, 88);
			DeviceCaps = GetDeviceCaps(v7, 90);
			ReleaseDC(0LL, v7);
		}
		else
		{
			DeviceCaps = 96;
		}
	}
	v9 = MulDiv(v1, DeviceCaps, 96);
	SendMessageW(*(HWND*)(a1 + 376 + extraOffset), 0x111Bu, v9, 0LL);
}

static __int64 __fastcall CNscTree_SetItemHeight(__int64 a1, int a2)
{
	int extraOffset = 0;

	if (g_osVersion.BuildNumber() >= 26100 || (g_osVersion.BuildNumber() >= 22621 && g_osVersion.BuildRevision() >= 4602)) // Ittr: Handle windows 11 offset difference. Inefficient but simplified code wasnt working
		extraOffset = 16;
	else if (g_osVersion.BuildNumber() >= 21996)
		extraOffset = 8;

	*(DWORD*)(a1 + 200 + extraOffset) = a2;
	CNscTree_ScaleAndSetRowHeight(a1 - 256);
	return 0LL;
}

extern HRESULT(__fastcall* CNSCHost_FillNSCOg)(uintptr_t nscHost);
static HRESULT __fastcall CNSCHost_FillNSC(uintptr_t nscHost) //todo: reimplement the filter from 7 shell32, CLSID_PersonalStartMenu GUID_2659b475_eeb8_48b7_8f07_b378810f48cf
{
	const int indentValue = 13;
	const int itemHeight = 19;

	bool isFilled = *(DWORD*)(nscHost + 0xCC);
	HRESULT result = CNSCHost_FillNSCOg(nscHost);

	if (!isFilled)
	{
		HMODULE modUser32 = GetModuleHandleW(L"User32.dll");

		fGetDpiForWindow = (decltype(fGetDpiForWindow))GetProcAddress(modUser32, "GetDpiForWindow");
		fGetWindowDpiAwarenessContext = (decltype(fGetWindowDpiAwarenessContext))GetProcAddress(modUser32, "GetWindowDpiAwarenessContext");
		fAreDpiAwarenessContextsEqual = (decltype(fAreDpiAwarenessContextsEqual))GetProcAddress(modUser32, "AreDpiAwarenessContextsEqual");

		INameSpaceTreeControl2* control = *(INameSpaceTreeControl2**)(nscHost + 0x70);

		//ideally we would want to queryinterface to get these, but im too lazy to get the guids for these, and i doubt these offsets would change
		IVisualProperties* visualProps = (IVisualProperties*)(__int64(control) + 0x20);
		INameSpaceTreeControlValuesPrivate* privatec = (INameSpaceTreeControlValuesPrivate*)(__int64(control) + 0x50);

		if (g_osVersion.BuildNumber() < 14393)
		{
			privatec->SetIndentValue(indentValue);
			visualProps->SetItemHeight(itemHeight);
		}
		else
		{
			CNscTree_SetIndentValue((uintptr_t)privatec, indentValue);
			CNscTree_SetItemHeight((uintptr_t)visualProps, itemHeight);
		}
	}
	
	return result;
}