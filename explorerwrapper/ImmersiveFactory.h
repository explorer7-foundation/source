#pragma once
#define INITGUID
#include "common.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_ImmersiveShell,0xc2f03a33, 0x21f5, 0x47fa, 0xb4,0xbb,0x15,0x63,0x62,0xa2,0xf2,0x39); //c2f03a33_21f5_47fa_b4bb_156362a2f239
DEFINE_GUID(IID_ImmersiveShellProvider,0x6D5140C1, 0x7436, 0x11CE, 0x80,0x34,0x00,0xaa,0x00,0x60,0x09,0xfa);

DEFINE_GUID(SID_IImmersiveMonitorService,0x47094E3A, 0x0CF2, 0x430F, 0x80,0x6F,0xCF,0x9E,0x4F,0x0F,0x12,0xDD); //{47094E3A-0CF2-430F-806F-CF9E4F0F12DD}
DEFINE_GUID(IID_IImmersiveMonitorService,0x4D4C1E64, 0xE410, 0x4FAA, 0xBA,0xFA,0x59,0xCA,0x06,0x9B,0xFE,0xC2); //{4D4C1E64-E410-4FAA-BAFA-59CA069BFEC2}

DEFINE_GUID(SID_IImmersiveLayout,0xE2304C77, 0xD2A6, 0x43AE, 0x82,0x40,0x08,0x7E,0x7E,0x51,0x0F,0xE8); //{E2304C77-D2A6-43AE-8240-087E7E510FE8}
DEFINE_GUID(IID_IImmersiveLayout,0xD770B2AD, 0x8F5E, 0x4B8E, 0xB3,0xDF,0xF0,0x5A,0x2A,0xB5,0x28,0x7C); //{D770B2AD-8F5E-4B8E-B3DF-F0 5A 2A B5 28 7C}

DEFINE_GUID(IID_IImmersiveMode,0x2814FACC, 0x5F7E, 0x43A5, 0x94,0x8C,0xC3,0xBD,0xC0,0xFF,0xEE,0x84); //{2814FACC-5F7E-43A5-948C-C3BDC0FFEE84}

DEFINE_GUID(IID_IIWpnPlatform,0x9FA045CB, 0xB9B3, 0x47BA, 0x84,0x2F,0xE2,0xAB,0x45,0x8F,0x2B,0x0C);

DEFINE_GUID(CLSID_PushNotificationPlatformCF,0x4655840e, 0xAB1A, 0x49D0, 0xA4,0xC4,0x26,0x1F,0xA1,0xC2,0x0E,0x86); //{4655840e-ab1a-49d0-a4c4-261fa1c20e86}
DEFINE_GUID(CLSID_PushNotificationPlatform,0x0C9281F9, 0x6DA1, 0x4006, 0x87,0x29,0xDE,0x6E,0x6B,0x61,0x58,0x1C);
#pragma endregion

class CImmersiveFactory : public IClassFactory
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

class CImmersiveProvider : public IServiceProvider
{
public:
	CImmersiveProvider();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IServiceProvider
	STDMETHODIMP QueryService( REFGUID guidService, REFIID riid, void **ppv );

private:
	long m_cRef;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveMonitorManager: public IUnknown
{
public:
	STDMETHOD(GetCount)(UINT*) PURE;
	STDMETHOD(GetConnectedCount)(UINT*) PURE;
	STDMETHOD(GetAt)(UINT,IUnknown**) PURE;
	STDMETHOD(GetFromHandle)(HMONITOR,IUnknown**) PURE;
	STDMETHOD(GetFromIdentity)(ULONG,IUnknown**) PURE;
	STDMETHOD(GetImmersiveProxyMonitor)(IUnknown**) PURE;
	STDMETHOD(QueryService)(HMONITOR, REFGUID guidService, REFIID riid, void **ppv ) PURE;
	STDMETHOD(QueryServiceByIdentity)(ULONG, REFGUID guidService, REFIID riid, void **ppv ) PURE;
	STDMETHOD(QueryServiceFromWindow)(HWND, REFGUID guidService, REFIID riid, void **ppv ) PURE;
	STDMETHOD(QueryServiceFromPoint)(tagPOINT*, REFGUID guidService, REFIID riid, void **ppv ) PURE;
	STDMETHOD(MoveImmersiveMonitor)(int) PURE;
	STDMETHOD(SetImmersiveMonitor)(IUnknown*) PURE;
};

class CImmersiveMonitorManager : public IImmersiveMonitorManager
{
public:
	CImmersiveMonitorManager();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IImmersiveMonitorManager
	STDMETHODIMP GetCount(UINT*);
	STDMETHODIMP GetConnectedCount(UINT*);
	STDMETHODIMP GetAt(UINT,IUnknown**);
	STDMETHODIMP GetFromHandle(HMONITOR,IUnknown**);
	STDMETHODIMP GetFromIdentity(ULONG,IUnknown**);
	STDMETHODIMP GetImmersiveProxyMonitor(IUnknown**);
	STDMETHODIMP QueryService(HMONITOR, REFGUID guidService, REFIID riid, void **ppv );
	STDMETHODIMP QueryServiceByIdentity(ULONG, REFGUID guidService, REFIID riid, void **ppv );
	STDMETHODIMP QueryServiceFromWindow(HWND, REFGUID guidService, REFIID riid, void **ppv );
	STDMETHODIMP QueryServiceFromPoint(tagPOINT*, REFGUID guidService, REFIID riid, void **ppv );
	STDMETHODIMP MoveImmersiveMonitor(int);
	STDMETHODIMP SetImmersiveMonitor(IUnknown*);
private:
	long m_cRef;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveLayout: public IUnknown
{
public:
	STDMETHOD(RegisterLayoutClient)(UINT,IUnknown*,ULONG*) PURE;
	STDMETHOD(UnregisterLayoutClient)(ULONG) PURE;
	STDMETHOD(RegisterForLayoutChanges)(UINT,IUnknown*,ULONG*) PURE;
	STDMETHOD(UnregisterForLayoutChanges)(ULONG) PURE;
	STDMETHOD(GetOuterWorkAreaForBand)(ULONG,tagRECT*) PURE;
	STDMETHOD(GetInnerWorkAreaForBand)(ULONG,tagRECT*) PURE;
	STDMETHOD(GetImmersiveShellWorkArea)(tagRECT*) PURE;
	STDMETHOD(InvalidateWorkArea)(ULONG) PURE;
	STDMETHOD(GetBandWorkAreaCount)(void) PURE;
	STDMETHOD(GetBandWorkAreaAt)(UINT,IUnknown**) PURE;
};

class CImmersiveLayout : public IImmersiveLayout
{
public:
	CImmersiveLayout(HMONITOR hMonitor);

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IImmersiveMonitorManager
	STDMETHODIMP RegisterLayoutClient(UINT,IUnknown*,ULONG*);
	STDMETHODIMP UnregisterLayoutClient(ULONG);
	STDMETHODIMP RegisterForLayoutChanges(UINT,IUnknown*,ULONG*);
	STDMETHODIMP UnregisterForLayoutChanges(ULONG);
	STDMETHODIMP GetOuterWorkAreaForBand(ULONG,tagRECT*);
	STDMETHODIMP GetInnerWorkAreaForBand(ULONG,tagRECT*);
	STDMETHODIMP GetImmersiveShellWorkArea(tagRECT*);
	STDMETHODIMP InvalidateWorkArea(ULONG);
	STDMETHODIMP GetBandWorkAreaCount(void);
	STDMETHODIMP GetBandWorkAreaAt(UINT,IUnknown**);
private:
	HMONITOR m_hMonitor;
	long m_cRef;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveMode: public IUnknown
{
public:
	STDMETHOD(GetMode)(DWORD*) PURE;
	STDMETHOD(SetMode)(DWORD) PURE;
};

class CImmersiveMode : public IImmersiveMode
{
public:
	CImmersiveMode();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IImmersiveMonitorManager
	STDMETHODIMP GetMode(DWORD*);
	STDMETHODIMP SetMode(DWORD);
private:
	long m_cRef;
};

void RegisterFakeImmersive();
void UnregisterFakeImmersive();

