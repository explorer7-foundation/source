#include "TrayNotify.h"
#include <Shlwapi.h>
#include "dbgprint.h"
#include "OSVersion.h"

const LPWSTR sz_TrayNotify = L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify";
const LPWSTR sz_TrayNotify7 = L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify7";

static BOOL verchecked;

extern "C" IStream* WINAPI SHOpenRegStream2WNEW(
  _In_      HKEY hkey,
  _In_opt_  LPCWSTR pszSubkey,
  _In_opt_  LPCWSTR pszValue,
  _In_      DWORD grfMode
)
{
	if (lstrcmp(pszSubkey,sz_TrayNotify) == 0)
	{		
		pszSubkey = sz_TrayNotify7;
		//wipe cache
		if (!verchecked)
		{
			WCHAR ourpath[MAX_PATH];
			WCHAR regpath[MAX_PATH];
			LONG regsz = MAX_PATH;
			GetModuleFileName(NULL,ourpath,MAX_PATH);
			if ( RegQueryValue(hkey,pszSubkey,regpath,&regsz) != ERROR_SUCCESS || lstrcmpi(ourpath,regpath) != 0 )
			{
				RegDeleteKey(hkey,pszSubkey);
				RegSetValue(hkey,pszSubkey,REG_SZ,ourpath,lstrlen(ourpath));
				dbgprintf(L"wiped traynotify cache");
			}
			verchecked = TRUE;
		}
	}
	return SHOpenRegStream2(hkey,pszSubkey,pszValue,grfMode);
}

/*CTRAYNOTIFYFACTORY*/

CTrayNotifyFactory::CTrayNotifyFactory(IClassFactory* origfactory)
{
	m_cRef = 1;
	m_origfactory = origfactory;
}

CTrayNotifyFactory::~CTrayNotifyFactory()
{
	m_origfactory->Release();
}

HRESULT STDMETHODCALLTYPE CTrayNotifyFactory::QueryInterface(REFIID riid,void **ppvObject)
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

ULONG STDMETHODCALLTYPE CTrayNotifyFactory::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CTrayNotifyFactory::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		m_origfactory->Release();
		free((void*)this);
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CTrayNotifyFactory::CreateInstance( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject )
{
	if ( pUnkOuter ) return CLASS_E_NOAGGREGATION;
	if (riid == IID_IUnknown)
	{
		IUnknown* obj;
		HRESULT ret = m_origfactory->CreateInstance(pUnkOuter,IID_IUnknown,(PVOID*)&obj);
		if (ret == S_OK)
		{
			ITrayNotify7* oldnotify;
			ret = obj->QueryInterface(IID_ITrayNotify7,(PVOID*)&oldnotify);
			if (ret == S_OK)
				*ppvObject = new CTrayNotifyWrapper(oldnotify);
		}
		return ret;
	}
	return E_FAIL; //BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG
}

HRESULT STDMETHODCALLTYPE CTrayNotifyFactory::LockServer( BOOL fLock )
{
	return m_origfactory->LockServer(fLock);
}

/*CTRAYNOTIFYWRAPPER*/

CTrayNotifyWrapper::CTrayNotifyWrapper(ITrayNotify7* notify7)
{
	m_cRef = 1;
	m_notify7 = notify7;
}

CTrayNotifyWrapper::~CTrayNotifyWrapper()
{	
	m_notify7->Release();
}

HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_ITrayNotify7)
	{
		*ppvObject = static_cast<ITrayNotify8*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_ITrayNotify8)
	{
		*ppvObject = static_cast<ITrayNotify8*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CTrayNotifyWrapper::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CTrayNotifyWrapper::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::RegisterCallback(IUnknown* p1,ULONG* p2)
{
	*p2 = 0;
	if (g_osVersion.BuildNumber() >= 10240)
		return S_OK;
	//INotificationCB* cb;
	//p1->QueryInterface(IID_PPV_ARGS(&cb));
	return m_notify7->RegisterCallback(p1);
}

HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::UnregisterCallback(ULONG)
{
	return E_NOTIMPL;
}
	
HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::SetPreference(PVOID* p1)
{
	return m_notify7->SetPreference(p1);
}

HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::EnableAutoTray(int p1)
{
	return m_notify7->EnableAutoTray(p1);
}

HRESULT STDMETHODCALLTYPE CTrayNotifyWrapper::DoAction(PVOID*,int)
{
	dbgprintf(L"DOACTION");
	return E_NOTIMPL;
}

HRESULT __stdcall CTrayNotifyWrapper::SetWindowingEnvironmentConfig(IUnknown*)
{
	dbgprintf(L"SetWindowingEnvironmentConfig");
	return E_NOTIMPL;
}
