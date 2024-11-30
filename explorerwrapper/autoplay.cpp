#include "autoplay.h"
#include "dbgprint.h"

CAutoPlayWrapper::CAutoPlayWrapper(IAutoPlayUI *autoui)
{
	m_cRef = 1;
	m_autoui = autoui;
}

CAutoPlayWrapper::~CAutoPlayWrapper()
{
	m_autoui->Release();
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::QueryInterface(REFIID riid,void **ppvObject)
{
	return m_autoui->QueryInterface(riid,ppvObject);
}

ULONG STDMETHODCALLTYPE CAutoPlayWrapper::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CAutoPlayWrapper::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::InitVolumeAutoplay(IUnknown * p1,LPCWSTR p2,LPCWSTR p3,ULONG p4,ULONG p5,ULONG p6,LPCWSTR p7,LPCWSTR p8,int p9,LPCWSTR p10,LPCWSTR p11,HWND p12)
{
	p6 = p6 | 2;
	return m_autoui->InitVolumeAutoplay(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::InitNoContentAutoplay(IUnknown * p1,REFGUID p2,LPCWSTR p3,ULONG p4,int p5,LPCWSTR p6,LPCWSTR p7,LPCWSTR p8)
{
	return m_autoui->InitNoContentAutoplay(p1,p2,p3,p4,p5,p6,p7,p8);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::InitDirectAutoPlay(IUnknown * p1,LPCWSTR p2,HWND p3)
{
	return m_autoui->InitDirectAutoPlay(p1,p2,p3);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::ToastPromptForChkDsk(LPCWSTR p1,int * p2,int * p3)
{
	return m_autoui->ToastPromptForChkDsk(p1,p2,p3);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::LaunchDeviceHandler(LPCWSTR p1,LPCWSTR p2,LPCWSTR p3)
{
	return m_autoui->LaunchDeviceHandler(p1,p2,p3);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::IsDialogClosed(void)
{
	return m_autoui->IsDialogClosed();
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::SniffComplete(ULONG p1)
{
	return m_autoui->SniffComplete(p1);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::CloseDialog(void)
{
	return m_autoui->CloseDialog();
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::AddContentType(ULONG p1)
{
	return m_autoui->AddContentType(p1);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::MoreInterfaceArrived(LPCWSTR p1)
{
	return m_autoui->MoreInterfaceArrived(p1);
}

HRESULT STDMETHODCALLTYPE CAutoPlayWrapper::SetChkDskCompleted(void)
{
	return m_autoui->SetChkDskCompleted();
}


