#pragma once
#define INITGUID
#include "common.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_TrayNotify,0x25DEAD04, 0x1EAC, 0x4911, 0x9e,0x3a,0xad,0x0a,0x4a,0xb5,0x60,0xfd); //
DEFINE_GUID(IID_ITrayNotify7,0xfb852b2c, 0x6bad, 0x4605, 0x95,0x51,0xf1,0x5f,0x87,0x83,0x09,0x35); //fb852b2c_6bad_4605_9551_f1 5f 87 83 09 35
DEFINE_GUID(IID_ITrayNotify8,0xd133ce13, 0x3537, 0x48ba, 0x93,0xa7,0xaf,0xcd,0x5d,0x20,0x53,0xb4); //d133ce13_3537_48ba_93a7_af cd 5d 20 53 b4
#pragma endregion

MIDL_INTERFACE("fb852b2c-6bad-4605-9551-f15f87830935")
ITrayNotify7: public IUnknown
{
public:
	STDMETHOD(RegisterCallback)(IUnknown*) PURE;
	STDMETHOD(SetPreference)(PVOID*) PURE;
	STDMETHOD(EnableAutoTray)(int) PURE;
};

MIDL_INTERFACE("d133ce13-3537-48ba-93a7-afcd5d2053b4")
ITrayNotify8: public IUnknown
{
public:
	STDMETHOD(RegisterCallback)(IUnknown*,ULONG*) PURE;
	STDMETHOD(UnregisterCallback)(ULONG) PURE;
	STDMETHOD(SetPreference)(PVOID*) PURE;
	STDMETHOD(EnableAutoTray)(int) PURE;
	STDMETHOD(DoAction)(PVOID*,int) PURE;
	STDMETHOD(SetWindowingEnvironmentConfig)(IUnknown*) PURE;
};

MIDL_INTERFACE("d782ccba-afb0-43f1-94db-fda3779eaccb")
INotificationCB : IUnknown
{
	STDMETHOD(Notify)(DWORD dwMessage, void* pNotifyItem) PURE;
};


class CTrayNotifyFactory : public IClassFactory
{
public:
	CTrayNotifyFactory(IClassFactory* origfactory);
	~CTrayNotifyFactory();

	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IClassFactory
	STDMETHODIMP CreateInstance( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject );
	STDMETHODIMP LockServer( BOOL fLock );
private:
	IClassFactory* m_origfactory;
	long m_cRef;
};

class CTrayNotifyWrapper : public ITrayNotify8
{
public:
	CTrayNotifyWrapper(ITrayNotify7* notify7);
	~CTrayNotifyWrapper();

	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
	STDMETHODIMP_(ULONG) AddRef( void);
	STDMETHODIMP_(ULONG) Release( void);

	//ITrayNotify8
	STDMETHODIMP RegisterCallback(IUnknown*,ULONG*);
	STDMETHODIMP UnregisterCallback(ULONG);
	STDMETHODIMP SetPreference(PVOID*);
	STDMETHODIMP EnableAutoTray(int);
	STDMETHODIMP DoAction(PVOID*,int);
	STDMETHODIMP SetWindowingEnvironmentConfig(IUnknown*);
private:
	ITrayNotify7* m_notify7;
	long m_cRef;
};
