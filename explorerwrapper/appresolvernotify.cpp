#include "appresolvernotify.h"
#include "dbgprint.h"

CAppResolverNotify8::CAppResolverNotify8(IAppResolverNotify7 *oldresolver)
{
	m_cRef = 1; //?
	m_resolver7 = oldresolver;
}

CAppResolverNotify8::~CAppResolverNotify8()
{
	m_resolver7->Release();
}

HRESULT STDMETHODCALLTYPE CAppResolverNotify8::QueryInterface(REFIID riid,void **ppvObject)
{
	dbgprintf(L"CAppResolverNotify8::QueryInterface NOT IMPLEMENTED");
	return E_NOTIMPL;
}

ULONG STDMETHODCALLTYPE CAppResolverNotify8::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CAppResolverNotify8::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CAppResolverNotify8::OnChangeNotify(long p1,PVOID* p2,PVOID* p3)
{	
	dbgprintf(L"CAppResolverNotify8::OnChangeNotify %p %p %p",p1,p2,p3);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAppResolverNotify8::AddHotkey(unsigned int p1, PVOID* p2, PVOID* p3, int p4)
{
	dbgprintf(L"CAppResolverNotify8::AddHotkey %p %p %p %p",p1,p2,p3,p4);
	return E_NOTIMPL;
	//return m_resolver7->AddHotkey(p1,p2,p3,p4);
}
