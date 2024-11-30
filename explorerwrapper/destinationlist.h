#pragma once
#define INITGUID
#include "framework.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_AutomaticDestinationList, 
	0x0F0AE1542, 0x0F497, 0x484B, 0xA1, 0x75, 0xA2, 0x0D, 0xB0, 0x91, 0x44, 0xBA);
DEFINE_GUID(IID_AutoDestList,
	0xBC10DCE3, 0x62F2, 0x4BC6, 0xAF, 0x37, 0xDB, 0x46, 0xED, 0x78, 0x73, 0xC4);
DEFINE_GUID(IID_AutoDestList10,
	0xE9C5EF8D, 0xFD41, 0x4F72, 0xBA, 0x87, 0xEB, 0x03, 0xBA, 0xD5, 0x81, 0x7C);

DEFINE_GUID(IID_CustomDestList, 0x03f1eed2, 0x8676, 0x430b, 0xab, 0xe1, 0x76, 0x5c, 0x1d, 0x8f, 0xe1, 0x47);
DEFINE_GUID(IID_CustomDestList10, 0x507101cd, 0xf6ad, 0x46c8, 0x8e, 0x20, 0xee, 0xb9, 0xe6, 0xba, 0xc4, 0x7f);
DEFINE_GUID(IID_CustomDestList1507, 0xfebd543d, 0x1f7b, 0x4b38, 0x94, 0x0b, 0x59, 0x33, 0xbd, 0x2c, 0xb2, 0x1b);
#pragma endregion

MIDL_INTERFACE("bc10dce3-62f2-4bc6-af37-db46ed7873c4")
IAutoDestinationList: public IUnknown
{
	STDMETHOD(Initialize)(LPCWSTR,LPCWSTR,LPCWSTR) PURE;
	STDMETHOD(HasList)(int*) PURE;
	STDMETHOD(GetList)(int, unsigned int, REFGUID, void**) PURE;
	STDMETHOD(AddUsagePoint)(IUnknown*) PURE;
	STDMETHOD(PinItem)(IUnknown*, int) PURE;
	STDMETHOD(IsPinned)(IUnknown*, int*) PURE;
	STDMETHOD(RemoveDestination)(IUnknown*) PURE;
	STDMETHOD(SetUsageData)(IUnknown*, float*, FILETIME*) PURE;
	STDMETHOD(GetUsageData)(IUnknown*, float*, FILETIME*) PURE;
	STDMETHOD(ResolveDestination)(HWND, ULONG, IShellItem*, REFGUID, void**) PURE;
	STDMETHOD(ClearList)(int) PURE;
};


MIDL_INTERFACE("e9c5ef8d-fd41-4f72-ba87-eb03bad5817c")
IAutoDestinationList10: public IUnknown
{
	STDMETHOD(Initialize)(LPCWSTR,LPCWSTR,LPCWSTR) PURE;
	STDMETHOD(HasList)(int*) PURE;
	STDMETHOD(GetList)(int, unsigned int, int, REFGUID, void**) PURE;
	STDMETHOD(AddUsagePoint)(IUnknown*) PURE;
	STDMETHOD(PinItem)(IUnknown*, int) PURE;
	STDMETHOD(IsPinned)(IUnknown*, int*) PURE;
	STDMETHOD(RemoveDestination)(IUnknown*) PURE;
	STDMETHOD(SetUsageData)(IUnknown*, float*, FILETIME*) PURE;
	STDMETHOD(GetUsageData)(IUnknown*, float*, FILETIME*) PURE;
	STDMETHOD(ResolveDestination)(HWND, unsigned long, IShellItem*, REFGUID, void**) PURE;
	STDMETHOD(ClearList)(int) PURE;
	STDMETHOD(AddUsagePointsEx)(IUnknown*, int, int) PURE;
	STDMETHOD(BlockItem)(IUnknown*) PURE;
	STDMETHOD(ClearBlocked)(void) PURE;
	STDMETHOD(TransferPoints)(IUnknown*, IUnknown*) PURE;
	STDMETHOD(HasListEx)(int*, int*) PURE;
	STDMETHOD(SetUsageDataInternal)(IUnknown*, float*, FILETIME*, int) PURE;
	STDMETHOD(GetUsageDataInternal)(IUnknown*, int, float*, FILETIME*, unsigned int*) PURE;
	STDMETHOD(UpdateRenamedItems)(IObjectCollection*, IObjectCollection*, int*) PURE;
	STDMETHOD(RemoveDeletedItems)(IObjectCollection*, int*) PURE;
	STDMETHOD(AddUsagePointsForFolders)(IObjectCollection*, int) PURE;
	STDMETHOD(UpdateCachedItems)(IObjectCollection*, int*) PURE;
};

MIDL_INTERFACE("03f1eed2-8676-430b-abe1-765c1d8fe147")
IInternalCustomDestList : public IUnknown
{
	STDMETHOD(SetMinItems)(UINT) PURE;
	STDMETHOD(SetApplicationID)(LPCWSTR) PURE;
	STDMETHOD(GetSlotCount)(UINT*) PURE;
	STDMETHOD(GetCategoryCount)(UINT*) PURE;
	STDMETHOD(GetCategory)(UINT, int, PVOID) PURE;
	STDMETHOD(DeleteCategory)(UINT, int) PURE;
	STDMETHOD(EnumerateCategoryDestinations)(UINT, REFIID, void**) PURE;
	STDMETHOD(RemoveDestination)(IUnknown*) PURE;
	STDMETHOD(ResolveDestination)(HWND, ULONG, IShellItem*, REFIID, void**) PURE;
};

MIDL_INTERFACE("507101cd-f6ad-46c8-8e20-eeb9e6bac47f")
IInternalCustomDestList10 : public IUnknown
{
	STDMETHOD(SetMinItems)(UINT) PURE;
	STDMETHOD(SetApplicationID)(LPCWSTR) PURE;
	STDMETHOD(GetSlotCount)(UINT*) PURE;
	STDMETHOD(GetCategoryCount)(UINT*) PURE;
	STDMETHOD(GetCategory)(UINT, int, PVOID) PURE;
	STDMETHOD(DeleteCategory)(UINT, int) PURE;
	STDMETHOD(EnumerateCategoryDestinations)(UINT, REFIID, void**) PURE;
	STDMETHOD(RemoveDestination)(IUnknown*) PURE;
	STDMETHOD(ResolveDestination)(HWND, ULONG, IShellItem*, REFIID, void**) PURE;
	STDMETHOD(HasListEx)(int*, int*) PURE;
	STDMETHOD(ClearRemovedDestinations)() PURE;
};

MIDL_INTERFACE("febd543d-1f7b-4b38-940b-5933bd2cb21b")
IInternalCustomDestList1507 : public IUnknown
{
	STDMETHOD(SetMinItems)(UINT) PURE;
	STDMETHOD(SetApplicationID)(LPCWSTR) PURE;
	STDMETHOD(GetSlotCount)(UINT*) PURE;
	STDMETHOD(GetCategoryCount)(UINT*) PURE;
	STDMETHOD(GetCategory)(UINT, int, PVOID) PURE;
	STDMETHOD(DeleteCategory)(UINT, int) PURE;
	STDMETHOD(EnumerateCategoryDestinations)(UINT, REFIID, void**) PURE;
	STDMETHOD(RemoveDestination)(IUnknown*) PURE;
	STDMETHOD(ResolveDestination)(HWND, ULONG, IShellItem*, REFIID, void**) PURE;
	STDMETHOD(HasListEx)(int*, int*) PURE;
};

class CAutoDestWrapper : public IAutoDestinationList
{
public:
	CAutoDestWrapper(IAutoDestinationList10*);
	~CAutoDestWrapper();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IAutoDestinationList
	STDMETHODIMP Initialize(LPCWSTR, LPCWSTR, LPCWSTR);
	STDMETHODIMP HasList(int*);
	STDMETHODIMP GetList(int, unsigned int, REFGUID, void**);
	STDMETHODIMP AddUsagePoint(IUnknown*);
	STDMETHODIMP PinItem(IUnknown*, int);
	STDMETHODIMP IsPinned(IUnknown*, int*);
	STDMETHODIMP RemoveDestination(IUnknown*);
	STDMETHODIMP SetUsageData(IUnknown*, float*, FILETIME*);
	STDMETHODIMP GetUsageData(IUnknown*, float*, FILETIME*);
	STDMETHODIMP ResolveDestination(HWND, unsigned long, IShellItem*, REFGUID, void**);
	STDMETHODIMP ClearList(int);

private:
	IAutoDestinationList10* m_dest10 = 0;
	long m_cRef;
};

class CCustomDestWrapper : IInternalCustomDestList
{
public:
	CCustomDestWrapper(IInternalCustomDestList10*);
	CCustomDestWrapper(IInternalCustomDestList1507*);
	~CCustomDestWrapper();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IInternalCustomDestList
	STDMETHODIMP SetMinItems(UINT);
	STDMETHODIMP SetApplicationID(LPCWSTR);
	STDMETHODIMP GetSlotCount(UINT*);
	STDMETHODIMP GetCategoryCount(UINT*);
	STDMETHODIMP GetCategory(UINT, int, PVOID);
	STDMETHODIMP DeleteCategory(UINT, int);
	STDMETHODIMP EnumerateCategoryDestinations(UINT, REFIID, void**);
	STDMETHODIMP RemoveDestination(IUnknown*);
	STDMETHODIMP ResolveDestination(HWND, ULONG, IShellItem*, REFIID, void**);

private:
	IInternalCustomDestList10* m_custDest10 = 0;
	IInternalCustomDestList1507* m_custDest1507 = 0;
	long m_cRef;
};