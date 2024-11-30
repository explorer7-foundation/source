#pragma once
#define INITGUID
#include "framework.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_AuthUIShutdownChoices,0x14CE31DC, 0xABc2, 0x484c, 0xb0,0x61,0xcf,0x34,0x16,0xae,0xd8,0xff); //{14CE31DC-ABC2-484C-B061-CF3416AED8FF}
DEFINE_GUID(IID_IShutdownChoices7, 0x0F678FCDE, 0x0EB44, 0x4B6E, 0x9B, 0x75, 0x0CC, 0x4A, 0x66, 0x1F, 0x52, 0x63); //f678fcde_eb44_4b6e_9b75_cc4a661f5263
DEFINE_GUID(IID_IShutdownChoices8,0x811CA537, 0x0FEEC, 0x4041, 0x9F, 0x51, 0x0E7, 0x0BD, 0x0A5, 0x96, 0x60, 0x1C); //811ca537_feec_4041_9f51_e7bda596601c
DEFINE_GUID(IID_IShutdownChoices10, 0xbfdc5e2f, 0x3402, 0x49b3, 0x87, 0x40, 0x91, 0xd6, 0xdc, 0x5d, 0xbb, 0x15);

DEFINE_GUID(IID_IAuthUILogonSound7, 0x9E1AA054, 0x1F71, 0x4C1E, 0x0AC, 0x0CC, 0x49, 0x4C, 0x5, 0x1F, 0x0B3, 0x39);
DEFINE_GUID(IID_IAuthUILogonSound10, 0x0C35243EA, 0x4CFC, 0x435A, 0x91, 0x0C2, 0x9D, 0x0BD, 0x0EC, 0x0BF, 0x0FC, 0x95);
#pragma endregion

MIDL_INTERFACE("f678fcde-eb44-4b6e-9b75-cc4a661f5263")
IShutdownChoices7: public IUnknown
{
public:
    STDMETHOD(Refresh)(void) PURE;
    STDMETHOD(CreateListener)(IUnknown**) PURE;
	STDMETHOD(SetChoiceMask)(ULONG) PURE;
	STDMETHOD(GetMessageWnd)(HWND**) PURE;
	STDMETHOD(SetShowBadChoices)(int) PURE;
	STDMETHOD(GetChoiceEnumerator)(IUnknown**) PURE;
	STDMETHOD(GetDefaultChoice)(ULONG*) PURE;
	STDMETHOD(UserHasShutdownRights)(void) PURE;
	STDMETHOD(GetChoiceName)(ULONG,int,LPWSTR,UINT) PURE;
	STDMETHOD(GetChoiceDesc)(ULONG,LPWSTR,UINT) PURE;
};

MIDL_INTERFACE("811ca537-feec-4041-9f51-e7bda596601c")
IShutdownChoices8: public IUnknown
{
public:
	STDMETHOD(Refresh)(void) PURE;
	STDMETHOD(CreateListener)(IUnknown**) PURE;
	STDMETHOD(SetChoiceMask)(ULONG) PURE;
	STDMETHOD(GetChoiceMask)(ULONG*) PURE;
	STDMETHOD(GetDefaultUIChoiceMask)(ULONG*) PURE;
	STDMETHOD(SetShowBadChoices)(int) PURE;
	STDMETHOD(GetChoiceEnumerator)(IUnknown**) PURE;
	STDMETHOD(GetDefaultChoice)(ULONG*) PURE;
	STDMETHOD(UserHasShutdownRights)(void) PURE;
	STDMETHOD(GetChoiceName)(ULONG,int,LPWSTR,UINT) PURE;
	STDMETHOD(GetChoiceDesc)(ULONG,LPWSTR,UINT) PURE;
};

MIDL_INTERFACE("bfdc5e2f-3402-49b3-8740-91d6dc5dbb15")
IShutdownChoices10: public IUnknown
{
public:
	// Custom methods
	STDMETHOD(Refresh)() PURE;
	STDMETHOD(SetChoiceMask)(ULONG choiceMask) PURE;
	STDMETHOD(GetChoiceMask)(ULONG* pChoiceMask) PURE;
	STDMETHOD(GetDefaultUIChoiceMask)(ULONG* pChoiceMask) PURE;
	STDMETHOD(SetShowBadChoices)(int showBadChoices) PURE;
	STDMETHOD(GetChoiceEnumerator)(IUnknown** ppEnum) PURE;
	STDMETHOD(GetDefaultChoice)(ULONG* pChoice) PURE;
	STDMETHOD(UserHasShutdownRights)() PURE;
	STDMETHOD(GetChoiceName)(ULONG choice, int index, LPWSTR nameBuffer, UINT bufferSize) PURE;
	STDMETHOD(GetChoiceDesc)(ULONG choice, LPWSTR descBuffer, UINT bufferSize) PURE;
};

class CAuthUIWrapper: public IShutdownChoices7
{
public:
	CAuthUIWrapper(IUnknown*, int build);
	~CAuthUIWrapper();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IShutdownChoices
	STDMETHODIMP Refresh(void);
	STDMETHODIMP CreateListener(IUnknown**);
	STDMETHODIMP SetChoiceMask(ULONG);
	STDMETHODIMP GetMessageWnd(HWND**);
	STDMETHODIMP SetShowBadChoices(int);
	STDMETHODIMP GetChoiceEnumerator(IUnknown**);
	STDMETHODIMP GetDefaultChoice(ULONG*);
	STDMETHODIMP UserHasShutdownRights(void);
	STDMETHODIMP GetChoiceName(ULONG,int,LPWSTR,UINT);
	STDMETHODIMP GetChoiceDesc(ULONG,LPWSTR,UINT);
private:
	IShutdownChoices8* m_authui8 = 0;
	IShutdownChoices10* m_authui10 = 0;
	long m_cRef;
};