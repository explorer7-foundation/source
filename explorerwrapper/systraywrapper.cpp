#include "systraywrapper.h"
#include "dbgprint.h"

CSysTrayWrapper::CSysTrayWrapper(IOleCommandTarget *stobject8)
{
	m_cRef = 1;
	m_stobject8 = stobject8;
	initialized = 0;
}

CSysTrayWrapper::~CSysTrayWrapper()
{
	m_stobject8->Release();
}

HRESULT STDMETHODCALLTYPE CSysTrayWrapper::QueryInterface(REFIID riid,void **ppvObject)
{
	dbgprintf(L"CSysTrayWrapper::QueryInterface NOT IMPLEMENTED");
	return E_NOTIMPL;
}

ULONG STDMETHODCALLTYPE CSysTrayWrapper::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CSysTrayWrapper::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CSysTrayWrapper::QueryStatus( const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[  ], OLECMDTEXT *pCmdText)
{	
	return m_stobject8->QueryStatus(pguidCmdGroup,cCmds,prgCmds,pCmdText);
}

HRESULT STDMETHODCALLTYPE CSysTrayWrapper::Exec( const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{	
	//command #2 differs 7->8; also command #4 must be issued later
	if (pguidCmdGroup && *pguidCmdGroup == CGID_ShellServiceObject && nCmdID == 2)
	{
		VARIANT v;
		InitVariantFromUInt32(0,&v);
		HRESULT ret = m_stobject8->Exec(pguidCmdGroup,2,0,&v,NULL);
		return ret;
	}
	else
		DoInitSysTray();

	HRESULT ret = m_stobject8->Exec(pguidCmdGroup,nCmdID,nCmdexecopt,pvaIn,pvaOut);
	return ret;
}

void CSysTrayWrapper::DoInitSysTray( void)
{	
	if (initialized) return;
	VARIANT v;
	InitVariantFromUInt32(3,&v);
	HRESULT ret = m_stobject8->Exec(&CGID_ShellServiceObject,4,0,&v,NULL); //start
	initialized = (ret == S_OK);
}