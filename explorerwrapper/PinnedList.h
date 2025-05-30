#pragma once
#include "common.h"
#include "OptionConfig.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_TaskbandPin, 0x90AA3A4E, 0x1CBA, 0x4233, 0xB8, 0xBB, 0x53, 0x57, 0x73, 0xD4, 0x84, 0x49);
DEFINE_GUID(IID_IPinnedList2, 0xBBD20037, 0xBC0E, 0x42F1, 0x91, 0x3F, 0xE2, 0x93, 0x6B, 0xB0, 0xEA, 0x0C);
DEFINE_GUID(IID_IPinnedList25, 0x446BC432, 0x57E9, 0x4B72, 0x8E, 0x0F1, 0x0AF, 0x27, 0x11, 0x3D, 0x0CF, 0x9C);
DEFINE_GUID(IID_IFlexibleTaskbarPinnedList, 0x60274fa2, 0x611f, 0x4b8a, 0xa2, 0x93, 0xf2, 0x7b, 0xf1, 0x03, 0xd1, 0x48);
DEFINE_GUID(IID_IPinnedList3, 0x0dd79ae2, 0xd156, 0x45d4, 0x9e, 0xeb, 0x3b, 0x54, 0x97, 0x69, 0xe9, 0x40);
DEFINE_GUID(SID_PinManager, 0xA5C8D635, 0xB4ED, 0x452B, 0x81, 0x09, 0x95, 0x01, 0x78, 0x10, 0x96, 0xD1);
#pragma endregion

MIDL_INTERFACE("BBD20037-BC0E-42F1-913F-E2936BB0EA0C")
IPinnedList2: public IUnknown
{
public:
	STDMETHOD(EnumObjects)(IEnumFullIDList**) PURE;
	STDMETHOD(Modify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetChangeCount)(ULONG*) PURE;
	STDMETHOD(GetPinnableInfo)(IDataObject*, int, IShellItem2**, IShellItem**, PWSTR*, INT*) PURE;
	STDMETHOD(IsPinnable)(IDataObject*, int) PURE;
	STDMETHOD(Resolve)(HWND, ULONG, PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(IsPinned)(PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetPinnedItem)(PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(GetAppIDForPinnedItem)(PCIDLIST_ABSOLUTE, PWSTR*) PURE;
	STDMETHOD(ItemChangeNotify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(UpdateForRemovedItemsAsNecessary)(VOID) PURE;
};

MIDL_INTERFACE("446bc432-57e9-4b72-8ef1-af27113dcf9c")
IPinnedList25: public IUnknown
{
public:
	STDMETHOD(EnumObjects)(IEnumFullIDList**) PURE;
	STDMETHOD(GetPinnableInfo)(IDataObject*, int, IShellItem2**, IShellItem**, PWSTR*, INT*) PURE;
	STDMETHOD(IsPinnable)(IDataObject*, int) PURE;
	STDMETHOD(Resolve)(HWND, ULONG, PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(Modify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetChangeCount)(ULONG*) PURE;
	STDMETHOD(IsPinned)(PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetPinnedItem)(PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(GetAppIDForPinnedItem)(PCIDLIST_ABSOLUTE, PWSTR*) PURE;
	STDMETHOD(ItemChangeNotify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(UpdateForRemovedItemsAsNecessary)(VOID) PURE;
};

MIDL_INTERFACE("60274FA2-611F-4B8A-A293-F27BF103D148")
IFlexibleTaskbarPinnedList : public IUnknown
{
public:
	STDMETHOD(EnumObjects)(IEnumFullIDList**) PURE;
	STDMETHOD(GetPinnableInfo)(IDataObject*, int, IShellItem2**, IShellItem**, PWSTR*, INT*) PURE;
	STDMETHOD(IsPinnable)(IDataObject*, int) PURE;
	STDMETHOD(Resolve)(HWND, ULONG, PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(Modify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetChangeCount)(ULONG*) PURE;
	STDMETHOD(IsPinned)(PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetPinnedItem)(PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(GetAppIDForPinnedItem)(PCIDLIST_ABSOLUTE, PWSTR*) PURE;
	STDMETHOD(ItemChangeNotify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(UpdateForRemovedItemsAsNecessary)(VOID) PURE;
	STDMETHOD(GetPinnedItemForAppID)(PWSTR, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(ApplyInPlaceTaskbarLayout)(int, int) PURE;
	STDMETHOD(ApplyReorderTaskbarLayout)(int, int) PURE;
};


MIDL_INTERFACE("0DD79AE2-D156-45D4-9EEB-3B549769E940")
IPinnedList3 : public IUnknown
{
public:
	STDMETHOD(EnumObjects)(IEnumFullIDList**) PURE;
	STDMETHOD(GetPinnableInfo)(IDataObject*, int, IShellItem2**, IShellItem**, PWSTR*, INT*) PURE;
	STDMETHOD(IsPinnable)(IDataObject*, int) PURE;
	STDMETHOD(Resolve)(HWND, ULONG, PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(Unadvise)(ULONG) PURE;
	STDMETHOD(GetChangeCount)(ULONG*) PURE;
	STDMETHOD(IsPinned)(PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(GetPinnedItem)(PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*) PURE;
	STDMETHOD(GetAppIDForPinnedItem)(PCIDLIST_ABSOLUTE, PWSTR*) PURE;
	STDMETHOD(ItemChangeNotify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(UpdateForRemovedItemsAsNecessary)(VOID) PURE;
	STDMETHOD(PinShellLink)(PWSTR, IShellLinkW*) PURE;
	STDMETHOD(GetPinnedItemForAppID)(PWSTR, PCIDLIST_ABSOLUTE) PURE;
	STDMETHOD(Modify)(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE, int) PURE;
};


enum PINNEDLISTMODIFYCALLER
{
	PMC_APPRESOLVERMIGRATION = 0,
	PMC_APPRESOLVERUNINSTALL = 1,
	PMC_APPRESOLVERUNPINUNIQUEID = 2,
	PMC_CONTENTDELIVERYMANAGERBROKER = 3,
	PMC_CONTEXTMENU = 4,
	PMC_DEFAULTMFUCHANGE = 5,
	PMC_DEFAULTMFUPIN = 6,
	PMC_DEFAULTMFUPINAUX = 7,
	PMC_DEFAULTMFUTRYPIN = 8,
	PMC_DEFAULTMFUUPGRADE = 9,
	PMC_IEXPLORERCOMMAND = 10,
	PMC_JUMPVIEWBROKER = 11,
	PMC_PINNEDLISTLAYOUT = 12,
	PMC_PINNEDLISTNONEXIST = 13,
	PMC_PINNEDLISTREORDERLAYOUT = 14,
	PMC_PINNEDLISTUNRESOLVE = 15,
	PMC_RETAILDEMO = 16,
	PMC_SHELLLINK = 17,
	PMC_STARTMENU = 18,
	PMC_STARTMNU = 19,
	PMC_TASKBANDBADSHORTCUT = 20,
	PMC_TASKBANDBROKENPIN = 21,
	PMC_TASKBANDDEDUPPIN = 22,
	PMC_TASKBANDINSERT = 23,
	PMC_TASKBANDMODIFY = 24,
	PMC_TASKBANDPIN = 25,
	PMC_TASKBANDPINGROUP = 26,
	PMC_TASKBANDREORDER = 27,
	PMC_TASKBARPINNABLESURFACEBROKER = 28,
	PMC_TASKBARPINNABLESURFACEBROKERMIGRATION = 29,
	PMC_TASKBARPINNINGBROKERFACTORY = 30,
	PMC_TESTCODE = 31,
	PMC_UNIFIEDTILEMODELBROKER = 32,
	PMC_TRIMOOBEPINS = 33,
	PMC_MICROSOFTEDGE = 34,
	PMC_UPDATESHORTCUT = 35,
	PMC_CLOUDDEFAULTLAYOUT = 36,
	PMC_BROWSERDECLUTTER = 37
};

enum TaskbarLayoutType
{
	TLT_0,
	TLT_1,
	TLT_2,
};

MIDL_INTERFACE("d75f625f-6df9-4874-970d-17cbf846f00d")
IPinManagerInterop : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE PinItemToTaskbarShim(PCUIDLIST_ABSOLUTE, PINNEDLISTMODIFYCALLER) = 0;
	virtual HRESULT STDMETHODCALLTYPE PinItemFromTrustedCaller(PCUIDLIST_ABSOLUTE, PINNEDLISTMODIFYCALLER) = 0;
	virtual HRESULT STDMETHODCALLTYPE ApplyPrependDefaultTaskbarLayout() = 0;
	virtual HRESULT STDMETHODCALLTYPE ApplyAppendDefaultTaskbarLayout() = 0; // @Warning: This appeared somewhere after the time of 22621.1992's release
	virtual HRESULT STDMETHODCALLTYPE ApplyInPlaceTaskbarLayout(TaskbarLayoutType) = 0;
	virtual HRESULT STDMETHODCALLTYPE ApplyReorderTaskbarLayout(TaskbarLayoutType, int) = 0;
};

MIDL_INTERFACE("87d9e034-56d0-4f8c-be59-997b01754710")
IPinManagerInterop2 : IPinManagerInterop
{
	virtual HRESULT STDMETHODCALLTYPE UnpinTaskbarItem(PCUIDLIST_ABSOLUTE, PINNEDLISTMODIFYCALLER) = 0;
	virtual HRESULT STDMETHODCALLTYPE UpdatePinnedTaskbarItem(PCUIDLIST_ABSOLUTE, PCUIDLIST_ABSOLUTE, PINNEDLISTMODIFYCALLER) = 0;
};

class CPinnedListWrapper : public IPinnedList2
{
public:
	CPinnedListWrapper(IUnknown*, int);
	~CPinnedListWrapper();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IPinnedList2
	STDMETHODIMP EnumObjects(IEnumFullIDList**);
	STDMETHODIMP Modify(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE);
	STDMETHODIMP GetChangeCount(ULONG*);
	STDMETHODIMP GetPinnableInfo(IDataObject*, int, IShellItem2**, IShellItem**, PWSTR*, INT*);
	STDMETHODIMP IsPinnable(IDataObject*, int);
	STDMETHODIMP Resolve(HWND, ULONG, PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*);
	STDMETHODIMP IsPinned(PCIDLIST_ABSOLUTE);
	STDMETHODIMP GetPinnedItem(PCIDLIST_ABSOLUTE, PIDLIST_ABSOLUTE*);
	STDMETHODIMP GetAppIDForPinnedItem(PCIDLIST_ABSOLUTE, PWSTR*);
	STDMETHODIMP ItemChangeNotify(PCIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE);
	STDMETHODIMP UpdateForRemovedItemsAsNecessary(VOID);
private:
	IFlexibleTaskbarPinnedList* m_flexList = 0;
	IPinnedList3* m_pinnedList3 = 0;
	IPinnedList25* m_pinnedList25 = 0;
	IPinManagerInterop* m_pinManager;

	int m_build = 0;
};
