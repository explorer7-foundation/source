#include "ProjectionFactory.h"
#include "dbgprint.h"
#pragma function(memset)

CProjectionFactory* CSingleProjection;

DWORD regid;

typedef enum _SECTION_INHERIT {
    ViewShare=1,
    ViewUnmap=2
} SECTION_INHERIT, *PSECTION_INHERIT;

static DWORD APCInject(LPWSTR sProcName,LPWSTR sDllName){
  DWORD dRet=0;
  //define type and pointer to function
  typedef NTSTATUS (WINAPI *tNtMapViewOfSection)(HANDLE,HANDLE,LPVOID,ULONG,SIZE_T,LARGE_INTEGER*,SIZE_T*,SECTION_INHERIT,ULONG,ULONG);
  tNtMapViewOfSection NtMapViewOfSection=(tNtMapViewOfSection)GetProcAddress(GetModuleHandle(L"ntdll.dll"),"NtMapViewOfSection");
  if(!NtMapViewOfSection)return -1;
  //create buffer
  HANDLE hFile=CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,(lstrlen(sDllName)*2)+1,NULL);
  if(!hFile)return -2;
  HANDLE hView=MapViewOfFile(hFile,FILE_MAP_ALL_ACCESS,0,0,0);
  if(!hView){
    CloseHandle(hFile);
    return -3;
  }else//set value to buffer
    lstrcpy((LPWSTR)hView,sDllName);
  // Starting target process
  PROCESS_INFORMATION pi;STARTUPINFO st;
  ZeroMemory(&pi,sizeof(pi));
  ZeroMemory(&st,sizeof(st));
  st.cb=sizeof(STARTUPINFO);
  //create suspended process
  if(CreateProcess(NULL,sProcName,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&st,&pi)){
    LPVOID RemoteString=NULL;SIZE_T ViewSize=0;
    if(NtMapViewOfSection(hFile,pi.hProcess,&RemoteString,0,0,NULL,&ViewSize,ViewShare,0,PAGE_READONLY)==0){
      LPVOID nLoadLibrary=(LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"LoadLibraryW");
      if(!QueueUserAPC((PAPCFUNC)nLoadLibrary,pi.hThread,(ULONG_PTR)RemoteString))
        dRet=-6;
    }else
      dRet=-5;
    ResumeThread(pi.hThread);	
    CloseHandle(pi.hThread);
	WaitForInputIdle(pi.hProcess,5000);
    CloseHandle(pi.hProcess);
  }else
    dRet=-4;
  UnmapViewOfFile(hView);
  CloseHandle(hFile);
  return dRet;
}

static BOOL RunDisplaySwitchInjected()
{
	WCHAR ourdll[MAX_PATH];
	WCHAR path[MAX_PATH];
	GetSystemDirectory(path,MAX_PATH);
	lstrcat(path,L"\\DisplaySwitch.exe -Embedding");
	GetModuleFileName(g_hInstance,ourdll,MAX_PATH);	
	return ( APCInject(path,ourdll) == 0 );
}

HRESULT STDMETHODCALLTYPE CProjectionFactory::QueryInterface(REFIID riid,void **ppvObject)
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

ULONG STDMETHODCALLTYPE CProjectionFactory::AddRef(void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE CProjectionFactory::Release(void)
{
	return 1;
}

HRESULT STDMETHODCALLTYPE CProjectionFactory::LockServer( BOOL fLock )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CProjectionFactory::CreateInstance( IUnknown * pUnkOuter, REFIID riid, void ** ppvObject )
{
	if ( pUnkOuter ) return CLASS_E_NOAGGREGATION;
	if (riid == IID_IUnknown)
	{
		*ppvObject = new CProjectionUI;
		return S_OK;
	}
	return E_NOINTERFACE;
}

CProjectionUI::CProjectionUI()
{
	m_cRef = 1;
}

HRESULT STDMETHODCALLTYPE CProjectionUI::QueryInterface(REFIID riid,void **ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_ProjectionUI)
	{
		*ppvObject = static_cast<IProjectionUI*>(this);
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CProjectionUI::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CProjectionUI::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CProjectionUI::CreateAndShow(IUnknown* p1)
{
	HRESULT res = E_NOTIMPL;
	UnregisterProjection();
	dbgprintf(L"CProjectionUI::CreateAndShow %p",p1);
	if ( RunDisplaySwitchInjected() )
	{
		IProjectionUI* newui;
		res = CoCreateInstance(CLSID_ProjectionUI,NULL,CLSCTX_LOCAL_SERVER,IID_ProjectionUI,(LPVOID*)&newui);
		if ( SUCCEEDED(res) )
		{
			newui->CreateAndShow(p1);
			newui->Release();
		}
	}
	RegisterProjection();
	return res;
}

HRESULT STDMETHODCALLTYPE CProjectionUI::Command(int p1,int p2)
{
	HRESULT res = E_NOTIMPL;
	UnregisterProjection();
	dbgprintf(L"CProjectionUI::Command %p %p",p1,p2);
	if ( RunDisplaySwitchInjected() )
	{
		IProjectionUI* newui;
		res = CoCreateInstance(CLSID_ProjectionUI,NULL,CLSCTX_LOCAL_SERVER,IID_ProjectionUI,(LPVOID*)&newui);
		if ( SUCCEEDED(res) )
		{
			newui->Command(p1,p2);
			newui->Release();
		}
	}
	RegisterProjection();
	return res;
}

void RegisterProjection()
{
	if (!CSingleProjection)
		CSingleProjection = new CProjectionFactory;
	CoRegisterClassObject(CLSID_ProjectionUI,CSingleProjection,CLSCTX_LOCAL_SERVER,REGCLS_SINGLEUSE,&regid); //not multiple
}

void UnregisterProjection()
{
	CoRevokeClassObject(regid);
}