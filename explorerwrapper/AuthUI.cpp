#include "AuthUI.h"
#include "dbgprint.h"

CAuthUIWrapper::CAuthUIWrapper(IUnknown* authui, int build)
{
	m_cRef = 1;
	if (build >= 10240)
		m_authui10 = (IShutdownChoices10*)authui;
	else
		m_authui8 = (IShutdownChoices8*)authui;
}

CAuthUIWrapper::~CAuthUIWrapper()
{
	if (m_authui8)
		m_authui8->Release();
	if (m_authui10)
		m_authui10->Release();
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
	if (m_authui8)
		return m_authui8->QueryInterface(riid, ppvObject);
	if (m_authui10)
		return m_authui10->QueryInterface(riid, ppvObject);
	return S_OK;
}

ULONG STDMETHODCALLTYPE CAuthUIWrapper::AddRef(void)
{
	if (m_authui8)
		m_authui8->AddRef();
	if (m_authui10)
		m_authui10->AddRef();
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CAuthUIWrapper::Release(void)
{
	if (m_authui8)
		m_authui8->Release();
	if (m_authui10)
		m_authui10->Release();
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		free((void*)this);
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::Refresh()
{
	if (m_authui8)
		return m_authui8->Refresh();
	if (m_authui10)
		return m_authui10->Refresh();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::CreateListener(IUnknown** p1)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::SetChoiceMask(ULONG p1)
{
	p1 = p1 & ~0x200000;
	dbgprintf(L"SetChoiceMask %p", p1);
	if (m_authui8)
		return m_authui8->SetChoiceMask(p1);
	if (m_authui10)
		return m_authui10->SetChoiceMask(p1);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::GetMessageWnd(HWND** p1)
{
	return E_NOTIMPL;
	//return m_authui->GetMessageWnd(p1);
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::SetShowBadChoices(int p1)
{
	if (m_authui8)
		return m_authui8->SetShowBadChoices(p1);
	if (m_authui10)
		return m_authui10->SetShowBadChoices(p1);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::GetChoiceEnumerator(IUnknown** p1)
{
	if (m_authui8)
		return m_authui8->GetChoiceEnumerator(p1);
	if (m_authui10)
		return m_authui10->GetChoiceEnumerator(p1);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::GetDefaultChoice(ULONG* p1)
{
	if (m_authui8)
		return m_authui8->GetDefaultChoice(p1);
	if (m_authui10)
		return m_authui10->GetDefaultChoice(p1);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::UserHasShutdownRights(void)
{
	if (m_authui8)
		return m_authui8->UserHasShutdownRights();
	if (m_authui10)
		return m_authui10->UserHasShutdownRights();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::GetChoiceName(ULONG p1, int p2, LPWSTR p3, UINT p4)
{
	dbgprintf(L"GetChoiceName %d %d %s %d", p1, p2, p3, p4);
	if (m_authui8)
		return m_authui8->GetChoiceName(p1, p2, p3, p4);
	if (m_authui10)
		return m_authui10->GetChoiceName(p1, p2, p3, p4);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CAuthUIWrapper::GetChoiceDesc(ULONG p1, LPWSTR p2, UINT p3)
{
	if (m_authui8)
		return  m_authui8->GetChoiceDesc(p1, p2, p3);
	if (m_authui10)
		return  m_authui10->GetChoiceDesc(p1, p2, p3);
	return S_OK;
}
