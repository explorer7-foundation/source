#pragma once
#define INITGUID
#include "framework.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_ProjectionUI,0xA19141CE, 0xD197, 0x4C8B, 0x82,0xC9,0x49,0x95,0xF5,0x30,0x34,0x97); //{A19141CE-D197-4C8B-82C9-4995F5303497}
DEFINE_GUID(IID_ProjectionUI,0x024bf402, 0x7bf2, 0x40e6, 0xb4,0xd2,0xca,0x5a,0x74,0x23,0xbb,0xd2); //024bf402_7bf2_40e6_b4d2_ca5a7423bbd2
#pragma endregion

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IProjectionUI: public IUnknown
{
public:
	STDMETHOD(CreateAndShow)(IUnknown*) PURE;
	STDMETHOD(Command)(int,int) PURE;
};

class CProjectionFactory : public IClassFactory
{
public:
	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
    STDMETHODIMP_(ULONG) AddRef( void);    
    STDMETHODIMP_(ULONG) Release( void);

	//IClassFactory
	STDMETHODIMP CreateInstance( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject );
	STDMETHODIMP LockServer( BOOL fLock );
};

class CProjectionUI : public IProjectionUI
{
public:
	CProjectionUI();

	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
    STDMETHODIMP_(ULONG) AddRef( void);    
    STDMETHODIMP_(ULONG) Release( void);

	//IProjectionUI
	STDMETHODIMP CreateAndShow(IUnknown*);
	STDMETHODIMP Command(int,int);
private:
	long m_cRef;
};

void RegisterProjection();
void UnregisterProjection();