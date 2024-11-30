#include "immersivefactory.h"
#include "dbgprint.h"

static DWORD dwRegisterImmersive;

HRESULT STDMETHODCALLTYPE CImmersiveFactory::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IClassFactory)
	{
		*ppvObject = static_cast<IClassFactory*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImmersiveFactory::AddRef(void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE CImmersiveFactory::Release(void)
{
	return 1;
}

HRESULT STDMETHODCALLTYPE CImmersiveFactory::CreateInstance( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject )
{
	if ( pUnkOuter ) return CLASS_E_NOAGGREGATION;
	if (riid == IID_IUnknown)
	{
		*ppvObject = new CImmersiveProvider;
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CImmersiveFactory::LockServer( BOOL fLock )
{
	return S_OK;
}

CImmersiveProvider::CImmersiveProvider()
{
	m_cRef = 1;
}

HRESULT STDMETHODCALLTYPE CImmersiveProvider::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_ImmersiveShellProvider)
	{
		*ppvObject = static_cast<IServiceProvider*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImmersiveProvider::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CImmersiveProvider::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

/*typedef BOOL (WINAPI *GetProcessUIContextInformationAPI)(HANDLE,DWORD*);
static GetProcessUIContextInformationAPI GetProcessUIContextInformation;

BOOL WINAPI GetProcessUIContextInformationNEW(HANDLE h,DWORD* out)
{
	BOOL ret = GetProcessUIContextInformation(h,out);
	dbgprintf(L"GetProcessUIContextInformation %p %p",h,*out);
	return ret;
}

PDWORD WINAPI GetSidSubAuthorityNEW( PSID pSid, DWORD nSubAuthority )
{
	PDWORD ret = GetSidSubAuthority(pSid,nSubAuthority);
	dbgprintf(L"GetSidSubAuthority = %p",ret);
	if (ret) dbgprintf(L"GetSidSubAuthority* = %p",*ret);
	return ret;
}*/

HRESULT STDMETHODCALLTYPE CImmersiveProvider::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
	if (guidService == SID_IImmersiveMonitorService && riid == IID_IImmersiveMonitorService)
	{
		*ppv = (PVOID*) new CImmersiveMonitorManager();
		return S_OK;
	}
	if (guidService == IID_IImmersiveMode && riid == IID_IImmersiveMode)
	{
		*ppv = (PVOID*) new CImmersiveMode();
		return S_OK;
	}
	/*if (guidService == IID_IIWpnPlatform && riid == IID_IIWpnPlatform)
	{		
		HMODULE hWpnApps = GetModuleHandle(L"wpnapps.dll");
		dbgprintf(L"wpnapps = %p",hWpnApps);
		GetProcessUIContextInformation = (GetProcessUIContextInformationAPI)GetProcAddress(GetModuleHandle(L"user32.dll"),(LPSTR)2521);
		ChangeImportedAddress(hWpnApps,"user32.dll",GetProcessUIContextInformation,GetProcessUIContextInformationNEW);
		ChangeImportedAddress(hWpnApps,"API-MS-WIN-SECURITY-BASE-L1-2-0.DLL",GetSidSubAuthority,GetSidSubAuthorityNEW);
		ChangeImportedAddress(hWpnApps,"API-MS-WIN-SECURITY-BASE-L1-2-0.DLL",GetProcAddress(GetModuleHandle(L"kenelbase.dll"),"GetSidSubAuthority"),GetSidSubAuthorityNEW);
		//"API-MS-WIN-SECURITY-BASE-L1-2-0.DLL"

		HRESULT res = CoCreateInstance(CLSID_PushNotificationPlatformCF,NULL,CLSCTX_INPROC_SERVER,riid,ppv);
		dbgprintf(L"Creating push notifications platform = %p",res);
		return res;
	}
	
	WCHAR clsid[40];
	WCHAR iid[40];
	StringFromGUID2(guidService,clsid,40);
	StringFromGUID2(riid,iid,40);
	dbgprintf(L"%p: CImmersiveProvider::QueryService %s [%s]",_ReturnAddress(),clsid,iid);*/
	return E_NOTIMPL;
}

CImmersiveMonitorManager::CImmersiveMonitorManager()
{
	m_cRef = 1;
}

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IImmersiveMonitorService)
	{
		*ppvObject = static_cast<IImmersiveMonitorManager*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImmersiveMonitorManager::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CImmersiveMonitorManager::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

#define UNIMPLFUNC dbgprintf(L"Unimplemented %S",__FUNCTION__); return E_NOTIMPL;

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetCount(UINT* p1)
{
	dbgprintf(L"CImmersiveMonitorManager::GetCount");
	*p1 = GetSystemMetrics(SM_CMONITORS);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetConnectedCount(UINT*){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetAt(UINT,IUnknown**){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetFromHandle(HMONITOR,IUnknown**){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetFromIdentity(ULONG,IUnknown**){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::GetImmersiveProxyMonitor(IUnknown**){UNIMPLFUNC}

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::QueryServiceByIdentity(ULONG, REFGUID guidService, REFIID riid, void **ppv ){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::QueryServiceFromWindow(HWND, REFGUID guidService, REFIID riid, void **ppv ){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::MoveImmersiveMonitor(int){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::SetImmersiveMonitor(IUnknown*){UNIMPLFUNC}

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::QueryServiceFromPoint(tagPOINT* pt, REFGUID guidService, REFIID riid, void **ppv )
{
	HMONITOR hMonitor = MonitorFromPoint(*pt,MONITOR_DEFAULTTOPRIMARY);
	return QueryService(hMonitor,guidService,riid,ppv);
}

HRESULT STDMETHODCALLTYPE CImmersiveMonitorManager::QueryService(HMONITOR hMonitor, REFGUID guidService, REFIID riid, void **ppv )
{
	if (guidService == SID_IImmersiveLayout && riid == IID_IImmersiveLayout)
	{
		*ppv = (PVOID*) new CImmersiveLayout(hMonitor);
		return S_OK;
	}
	/*WCHAR clsid[40];
	WCHAR iid[40];
	StringFromGUID2(guidService,clsid,40);
	StringFromGUID2(riid,iid,40);
	dbgprintf(L"CImmersiveMonitorManager::QueryService %p %s [%s]",_ReturnAddress(),hMonitor,clsid,iid);*/
	return E_NOTIMPL;
}

CImmersiveLayout::CImmersiveLayout(HMONITOR hMonitor)
{
	m_hMonitor = hMonitor;
	m_cRef = 1;
}

HRESULT STDMETHODCALLTYPE CImmersiveLayout::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IImmersiveLayout)
	{
		*ppvObject = static_cast<IImmersiveLayout*>(this);
		AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImmersiveLayout::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CImmersiveLayout::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CImmersiveLayout::RegisterLayoutClient(UINT,IUnknown*,ULONG*){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::UnregisterLayoutClient(ULONG){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::RegisterForLayoutChanges(UINT,IUnknown*,ULONG*){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::UnregisterForLayoutChanges(ULONG){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::GetImmersiveShellWorkArea(tagRECT*){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::InvalidateWorkArea(ULONG){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::GetBandWorkAreaCount(void){UNIMPLFUNC}
HRESULT STDMETHODCALLTYPE CImmersiveLayout::GetBandWorkAreaAt(UINT,IUnknown**){UNIMPLFUNC}

HRESULT STDMETHODCALLTYPE CImmersiveLayout::GetInnerWorkAreaForBand(ULONG,tagRECT* out)
{
	dbgprintf(L"CImmersiveLayout::GetInnerWorkAreaForBand");
	SetRectEmpty(out);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	if ( GetMonitorInfo(m_hMonitor,&mi) )
		CopyRect(out,&mi.rcWork);
	else
		SystemParametersInfo(SPI_GETWORKAREA,0,out,0);
		
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImmersiveLayout::GetOuterWorkAreaForBand(ULONG,tagRECT* out)
{
	return GetInnerWorkAreaForBand(0,out);
}

CImmersiveMode::CImmersiveMode()
{
	m_cRef = 1;
}

HRESULT STDMETHODCALLTYPE CImmersiveMode::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IImmersiveMode)
	{
		*ppvObject = static_cast<IImmersiveMode*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CImmersiveMode::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CImmersiveMode::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CImmersiveMode::GetMode(DWORD* mode)
{
	dbgprintf(L"CImmersiveMode::GetMode");
	*mode = 0; //ISM_DESKTOP
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CImmersiveMode::SetMode(DWORD mode)
{
	dbgprintf(L"CImmersiveMode::SetMode");
	return E_NOTIMPL;
}

void RegisterFakeImmersive()
{
	CoRegisterClassObject(CLSID_ImmersiveShell,new CImmersiveFactory,CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER,REGCLS_MULTIPLEUSE,&dwRegisterImmersive);
}

void UnregisterFakeImmersive()
{
	CoRevokeClassObject(dwRegisterImmersive);
}
