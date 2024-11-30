#include "framework.h"

extern "C" DWORD WINAPI SHGetSignature(DWORD p1, DWORD p2, DWORD p3)
{
	return E_NOTIMPL;
}

extern "C" DWORD WINAPI InitProcessPriv(DWORD unk1, HMODULE hInst, DWORD unk2, DWORD unk3)
{
	typedef HRESULT (WINAPI *InitProcessPriv8)(DWORD, HMODULE, DWORD, DWORD, DWORD);
	static InitProcessPriv8 IPP8;
	if (IPP8 == NULL)
		IPP8 = (InitProcessPriv8)GetProcAddress(LoadLibrary(L"DUI70.dll"),"InitProcessPriv");
	//like e8 does
	return IPP8(0xE,hInst,1,1,1);
}

extern "C" DWORD WINAPI SHInvokeCommandWithFlagsAndSite(DWORD_PTR unk1, DWORD_PTR unk2, DWORD_PTR unk3, DWORD_PTR unk4,DWORD unk5,DWORD_PTR unk6)
{
	typedef HRESULT (WINAPI *SHInvokeCommandWithFlagsAndSite8)(DWORD_PTR, DWORD_PTR, DWORD_PTR, DWORD_PTR, DWORD, DWORD_PTR, DWORD_PTR);
	static SHInvokeCommandWithFlagsAndSite8 SHInvoke8;
	if (SHInvoke8 == NULL)
		SHInvoke8 = (SHInvokeCommandWithFlagsAndSite8)GetProcAddress(GetModuleHandle(L"SHLWAPI.dll"),(LPSTR)571);

	return SHInvoke8(unk1,unk2,unk3,unk4,unk5,unk6,0);
}

//don't ask shell for ICommDlgBrowser. breaks search with stack overflow
extern "C" HRESULT WINAPI IUnknown_QueryServiceNEW( IUnknown *punk, REFGUID guidService, REFIID riid, void **ppvOut)
{
	if (guidService == IID_ICommDlgBrowser) return E_NOINTERFACE;
	return IUnknown_QueryService(punk,guidService,riid,ppvOut);
}

extern "C" void WINAPI WinListInit()
{
	typedef DWORD (WINAPI *STF)(DWORD,DWORD);
	STF SetThreadFlags = (STF)GetProcAddress(GetModuleHandle(L"shell32.dll"),(LPSTR)904);
	SetThreadFlags(1,1);
}

extern "C" void WINAPI WinListUninit()
{
	typedef DWORD (WINAPI *SUCH)();
	SUCH SkipDLLUnloadInitChecks = (SUCH)GetProcAddress(GetModuleHandle(L"dui70.dll"),"SkipDLLUnloadInitChecks");
	SkipDLLUnloadInitChecks();
}
