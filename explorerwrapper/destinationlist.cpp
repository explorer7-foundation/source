#include "destinationlist.h"

CAutoDestWrapper::CAutoDestWrapper(IAutoDestinationList10* destlist)
{
	m_cRef = 1;
	m_dest10 = destlist;
}

CAutoDestWrapper::~CAutoDestWrapper()
{
	m_dest10->Release();
}

HRESULT __stdcall CAutoDestWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
	return m_dest10->QueryInterface(riid, ppvObject);
}

ULONG __stdcall CAutoDestWrapper::AddRef(void)
{
	m_dest10->AddRef();
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CAutoDestWrapper::Release(void)
{
	m_dest10->Release();
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		free((void*)this);
		return 0;
	}
	return m_cRef;
}

HRESULT __stdcall CAutoDestWrapper::Initialize(LPCWSTR p1, LPCWSTR p2, LPCWSTR p3)
{
	return m_dest10->Initialize(p1, p2, p3);
}

HRESULT __stdcall CAutoDestWrapper::HasList(int* p1)
{
	return m_dest10->HasList(p1);
}

HRESULT __stdcall CAutoDestWrapper::GetList(int p1, unsigned int p2, REFGUID p3, void** p4)
{
	return m_dest10->GetList(p1, p2, 0, p3, p4);
}

HRESULT __stdcall CAutoDestWrapper::AddUsagePoint(IUnknown* p1)
{
	return m_dest10->AddUsagePoint(p1);
}

HRESULT __stdcall CAutoDestWrapper::PinItem(IUnknown* p1, int p2)
{
	return m_dest10->PinItem(p1, p2);
}

HRESULT __stdcall CAutoDestWrapper::IsPinned(IUnknown* p1, int* p2)
{
	return m_dest10->IsPinned(p1, p2);
}

HRESULT __stdcall CAutoDestWrapper::RemoveDestination(IUnknown* p1)
{
	return m_dest10->RemoveDestination(p1);
}

HRESULT __stdcall CAutoDestWrapper::SetUsageData(IUnknown* p1, float* p2, FILETIME* p3)
{
	return m_dest10->SetUsageData(p1, p2, p3);
}

HRESULT __stdcall CAutoDestWrapper::GetUsageData(IUnknown* p1, float* p2, FILETIME* p3)
{
	return m_dest10->GetUsageData(p1, p2, p3);
}

HRESULT __stdcall CAutoDestWrapper::ResolveDestination(HWND p1, unsigned long p2, IShellItem* p3, REFGUID p4, void** p5)
{
	return m_dest10->ResolveDestination(p1, p2, p3, p4, p5);
}

HRESULT __stdcall CAutoDestWrapper::ClearList(int p1)
{
	return m_dest10->ClearList(p1);
}



CCustomDestWrapper::CCustomDestWrapper(IInternalCustomDestList10* custDest)
{
	m_cRef = 1;
	m_custDest10 = custDest;
}

CCustomDestWrapper::CCustomDestWrapper(IInternalCustomDestList1507* custDest)
{
	m_cRef = 1;
	m_custDest1507 = custDest;
}

CCustomDestWrapper::~CCustomDestWrapper()
{
	if (m_custDest10)
		m_custDest10->Release();
	if (m_custDest1507)
		m_custDest1507->Release();
}

HRESULT __stdcall CCustomDestWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
	if (m_custDest10)
		return m_custDest10->QueryInterface(riid, ppvObject);
	if (m_custDest1507)
		return m_custDest1507->QueryInterface(riid, ppvObject);
	return S_OK;
}

ULONG __stdcall CCustomDestWrapper::AddRef(void)
{
	if (m_custDest10)
		m_custDest10->AddRef(); 
	if (m_custDest1507)
		m_custDest1507->AddRef();
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CCustomDestWrapper::Release(void)
{
	ULONG cRef = 0;
	if (m_custDest10)
		cRef = m_custDest10->Release();
	if (m_custDest1507)
		cRef = m_custDest1507->Release();
	if (InterlockedDecrement(&m_cRef) == 0 || cRef == 0)
	{
		free((void*)this);
		return 0;
	}
	return cRef;
}

HRESULT __stdcall CCustomDestWrapper::SetMinItems(UINT p1)
{
	if (m_custDest10)
		return m_custDest10->SetMinItems(p1);
	if (m_custDest1507)
		return m_custDest1507->SetMinItems(p1);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::SetApplicationID(LPCWSTR p1)
{
	if (m_custDest10)
		return m_custDest10->SetApplicationID(p1);
	if (m_custDest1507)
		return m_custDest1507->SetApplicationID(p1);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::GetSlotCount(UINT* p1)
{
	if (m_custDest10)
		return m_custDest10->GetSlotCount(p1);
	if (m_custDest1507)
		return m_custDest1507->GetSlotCount(p1);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::GetCategoryCount(UINT* p1)
{
	if (m_custDest10)
		return m_custDest10->GetCategoryCount(p1);
	if (m_custDest1507)
		return m_custDest1507->GetCategoryCount(p1);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::GetCategory(UINT p1, int p2, PVOID p3)
{
	if (m_custDest10)
		return m_custDest10->GetCategory(p1, p2, p3);
	if (m_custDest1507)
		return m_custDest1507->GetCategory(p1, p2, p3);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::DeleteCategory(UINT p1, int p2)
{
	if (m_custDest10)
		return m_custDest10->DeleteCategory(p1, p2);
	if (m_custDest1507)
		return m_custDest1507->DeleteCategory(p1, p2);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::EnumerateCategoryDestinations(UINT p1, REFIID p2, void** p3)
{
	if (m_custDest10)
		return m_custDest10->EnumerateCategoryDestinations(p1, p2, p3);
	if (m_custDest1507)
		return m_custDest1507->EnumerateCategoryDestinations(p1, p2, p3);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::RemoveDestination(IUnknown* p1)
{
	if (m_custDest10)
		return m_custDest10->RemoveDestination(p1);
	if (m_custDest1507)
		return m_custDest1507->RemoveDestination(p1);
	return S_OK;
}

HRESULT __stdcall CCustomDestWrapper::ResolveDestination(HWND p1, ULONG p2, IShellItem* p3, REFIID p4, void** p5)
{
	if (m_custDest10)
		return m_custDest10->ResolveDestination(p1, p2, p3, p4, p5);
	if (m_custDest1507)
		return m_custDest1507->ResolveDestination(p1, p2, p3, p4, p5);
	return S_OK;
}
