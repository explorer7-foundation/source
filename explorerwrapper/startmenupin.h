#ifndef _STARTMENUPIN_H
#define _STARTMENUPIN_H

#define INITGUID
#include "framework.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_StartMenuPin,0xA2A9545D, 0xA0C2, 0x42B4, 0x97,0x08,0xA0,0xB2,0xBA,0xDD,0x77,0xC8); //{A2A9545D-A0C2-42B4-9708-A0B2BADD77C8}
DEFINE_GUID(CLSID_TaskbarPin,0x90AA3A4E, 0x1CBA, 0x4233, 0xB8,0xBB,0x53,0x57,0x73,0xD4,0x84,0x49);
#pragma endregion

typedef HRESULT (WINAPI* CreateInstance_API)(PVOID,REFIID,PVOID*);
	typedef struct { 
		PVOID dunno1;
		DWORD_PTR dunno2;
		REFCLSID rclsid;
		CreateInstance_API CreateFunc; 
	} SHELLGUIDS, *PSHELLGUIDS; 

void StartMenuPin_PatchShell32();

/*MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IContextMenuShort
{
public:
	virtual void QueryInterface() = 0;
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryContextMenu( HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) = 0;
};*/

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IStartMenuShellExtInit
{
public:
	STDMETHOD_(void,QueryInterface)() PURE;
	STDMETHOD_(void, AddRef)() PURE;
	STDMETHOD_(void, Release)() PURE;
	STDMETHOD_(void, Initialize)() PURE;
	STDMETHOD_(LRESULT, SetChangeCount)(DWORD value) PURE;
	STDMETHOD_(IStream*, OpenPinRegStream)(DWORD grfMode) PURE;
	STDMETHOD_(IStream*, OpenLinksRegStream)(DWORD grfMode) PURE;
	STDMETHOD_(void, NotifyPinListChange)() PURE;
	STDMETHOD_(LRESULT,GetPinStreamVersion)() PURE;
	STDMETHOD_(LRESULT, SetPinStreamVersion)(DWORD value) PURE;
	STDMETHOD_(void, Unimpl1)() PURE;
	STDMETHOD_(void, UpgradeItem)() PURE;
	STDMETHOD_(LRESULT, GetBackupSubDirName)(LPWSTR szOut, UINT cbLen) PURE;
	STDMETHOD_(void, IsAcceptableTarget)() PURE;
	STDMETHOD_(DWORD, IsRestricted)() PURE;
	STDMETHOD_(void, Unimpl2)() PURE;
	STDMETHOD_(LRESULT, GetMenuStringID)(UINT* w) PURE;
	STDMETHOD_(int, GetHelpText)(unsigned __int64, LPWSTR, UINT) PURE;
	STDMETHOD_(LRESULT, GetChangeCount)(DWORD* pdwVal) PURE;
	STDMETHOD_(wchar_t*, GetVerb)(UINT op) PURE;
	STDMETHOD_(void, SendPinRearrangeSQM)() PURE;
	STDMETHOD_(LRESULT, SetRemovedChangeCount)(DWORD value) PURE;
	STDMETHOD_(LRESULT, GetRemovedChangeCount)() PURE;
	STDMETHOD_(void, GetPinnedAppSQMEventID)() PURE;
	STDMETHOD_(void, AppliesTo)() PURE;
	STDMETHOD_(void, v_GetPinListMutexName)() PURE;
};

class CStartMenuPin  : public IStartMenuShellExtInit/*, public IContextMenuShort*/
{
public:
	//constructor
	//CStartMenuPin();
	//destructor
	//~CStartMenuPin();
	void QueryInterface();
	void AddRef();
	void Release();
	void Initialize();
	LRESULT SetChangeCount(ULONG value);
	IStream* OpenPinRegStream(ULONG grfMode);
	IStream* OpenLinksRegStream(ULONG grfMode);
	void NotifyPinListChange();
	LRESULT GetPinStreamVersion();
	LRESULT SetPinStreamVersion(ULONG value);
	void Unimpl1();
	void UpgradeItem();
	LRESULT GetBackupSubDirName(LPWSTR szOut, UINT cbLen);
	void IsAcceptableTarget();
	DWORD IsRestricted();
	void Unimpl2();
	LRESULT GetMenuStringID(UINT* w);
	int GetHelpText(unsigned __int64,LPWSTR,UINT);
	LRESULT GetChangeCount(ULONG* pdwVal);
	wchar_t* GetVerb(UINT op);
	void SendPinRearrangeSQM();
	LRESULT SetRemovedChangeCount(ULONG value);
	LRESULT GetRemovedChangeCount();
	void GetPinnedAppSQMEventID();
	void AppliesTo();
	void v_GetPinListMutexName();
	/*HRESULT STDMETHODCALLTYPE QueryContextMenu( HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);*/
};

typedef HRESULT (__thiscall* GetMenuStringID_API)(PVOID,DWORD*);
/*typedef HRESULT (__thiscall* QueryContextMenu_API)(PVOID,HMENU,UINT,UINT,UINT,UINT);*/

typedef struct { 
	PVOID QueryInterface;
	PVOID AddRef;
	PVOID Release;
	PVOID Initialize;
	PVOID SetChangeCount;
	PVOID OpenPinRegStream;
	PVOID OpenLinksRegStream;
	PVOID NotifyPinListChange;
	PVOID GetPinStreamVersion;
	PVOID SetPinStreamVersion;
	PVOID Unimpl1;
	PVOID UpgradeItem;
	PVOID GetBackupSubDirName;
	PVOID IsAcceptableTarget;
	PVOID IsRestricted;
	PVOID Unimpl2;
	GetMenuStringID_API GetMenuStringID;
	PVOID GetHelpText;
	PVOID GetChangeCount;
	PVOID GetVerb;
	PVOID SendPinRearrangeSQM;
	PVOID SetRemovedChangeCount;
	PVOID GetRemovedChangeCount;
	PVOID GetPinnedAppSQMEventID; //on 10 it is IsDirty, but names shouldn't matter
	PVOID AppliesTo; //this and the one below dont exist on 8.1
	PVOID v_GetPinListMutexName;
} STARTPINVTBL, *PSTARTPINVTBL; 

typedef struct {
	PSTARTPINVTBL pStartPinVtbl;
} STARTPINOBJ, *PSTARTPINOBJ;


#endif