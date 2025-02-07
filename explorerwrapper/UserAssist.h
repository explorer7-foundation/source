#pragma once
#define INITGUID
#include "common.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_UserAssist,0xDD313E04, 0xFEFF, 0x11D1, 0x8E,0xCD,0x00,0x00,0xF8,0x7A,0x47,0x0C);
DEFINE_GUID(IID_IUserAssist7,0x90d75131, 0x43a6, 0x4664, 0x9a,0xf8,0xdc,0xce,0xb8,0x5a,0x74,0x62); //90d75131_43a6_4664_9af8_dcceb85a7462
DEFINE_GUID(IID_IUserAssist72,0x90d75131, 0x43a6, 0x4664, 0x9a,0xf8,0xdc,0xce,0xb8,0x5a,0x74,0x62); //90d75131_43a6_4664_9af8_dcceb85a7462
DEFINE_GUID(IID_IUserAssist10, 0x49B36D57, 0x5FD2, 0x45A7, 0x98, 0x1B, 0x6, 0x2, 0x8D, 0x57, 0x7A, 0x47); //49b36d57_5fd2_45a7_981b_06028d577a47
DEFINE_GUID(IID_IUserAssist102, 0x1F052FA3, 0x7A76, 0x4BEC, 0x96, 0x0C4, 0x0E8, 0x65, 0x0CF, 0x1B, 0x55, 0x0F1); //90d75131_43a6_4664_9af8_dcceb85a7462
DEFINE_GUID(UAIID_SHORTCUTS,0xF4E57C4B, 0x2036, 0x45f0, 0xa9,0xab,0x44,0x3b,0xcf,0xe3,0x3d,0x9f);
#pragma endregion

typedef struct tagUEMINFO {
	DWORD cbSize; /*  +0x0000  */
	DWORD dwMask; /*  +0x0004  */
	DWORD R; /*  +0x0008 40 00 00 00  */
	DWORD cLaunches; /*  +0x000c 75 00 00 00  */
	DWORD cSwitches; /*  +0x0010 75 00 00 00  */
	DWORD dwTime; /*  +0x0014  */
	FILETIME ftExecute; /*  +0x0018 ce 1a 00 00  */
	BOOL fExcludeFromMFU; /*  +0x0020 74 00 00 00  */
} UEMINFO, *PUEMINFO;

MIDL_INTERFACE("90d75131-43a6-4664-9af8-dcceb85a7462")
IUserAssist: public IUnknown
{
public:
    STDMETHOD(FireEvent)(REFIID, PVOID, LPWSTR, int) PURE;
	STDMETHOD(QueryEntry)(REFIID, LPWSTR, PUEMINFO) PURE;
	//...more of whatever
};

HRESULT WINAPI UAQueryEntry(REFIID iid, LPWSTR parsingname, PUEMINFO uem);
HRESULT WINAPI UAQueryShortcut(LPITEMIDLIST pidl, PUEMINFO uem);
LPWSTR WINAPI CoAllocString(LPWSTR src);