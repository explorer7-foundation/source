#include "shellurl.h"

CShellURLWrapper::CShellURLWrapper(IShellURL10* actual)
{
	m_cRef = 0;
	m_actual = actual;
}

HRESULT CShellURLWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
	return m_actual->QueryInterface(riid, ppvObject);
}

ULONG CShellURLWrapper::AddRef(void)
{
	if (m_actual)
		m_actual->AddRef();
	return InterlockedIncrement(&m_cRef);
}

ULONG CShellURLWrapper::Release(void)
{
	m_actual->Release();
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		free((void*)this);
		return 0;
	}
	return m_cRef;
}

HRESULT CShellURLWrapper::ParseFromOutsideSource(ushort const* a1, ulong a2)
{
	return m_actual->ParseFromOutsideSource(a1, a2);
}

HRESULT CShellURLWrapper::GetUrl(ushort* a1, ulong a2)
{
	return m_actual->GetUrl(a1, a2);
}

HRESULT CShellURLWrapper::SetUrl(ushort const* a1, ulong a2)
{
	return m_actual->SetUrl(a1, a2);
}

HRESULT CShellURLWrapper::GetDisplayName(ushort* a1, ulong a2)
{
	return m_actual->GetDisplayName(a1, a2);
}

HRESULT CShellURLWrapper::GetPidl(LPITEMIDLIST* a1)
{
	return m_actual->GetPidl(a1);
}

HRESULT CShellURLWrapper::SetPidl(LPITEMIDLIST a1)
{
	return m_actual->SetPidl(a1);
}

HRESULT CShellURLWrapper::SetPidlAndArgs(LPITEMIDLIST a1, ushort const* a2)
{
	return m_actual->SetPidlAndArgs(a1, a2);
}

HRESULT CShellURLWrapper::GetArgs(void)
{
	return m_actual->GetArgs();
}

HRESULT CShellURLWrapper::AddPath(LPITEMIDLIST a1)
{
	return m_actual->AddPath(a1);
}

HRESULT CShellURLWrapper::SetCancelObject(void* a1)
{
	return m_actual->SetCancelObject(a1);
}

HRESULT CShellURLWrapper::StartAsyncPathParse(HWND__* a1, ushort const* a2, ulong a3, void* a4)
{
	return m_actual->StartAsyncPathParse(a1, a2, a3, a4);
}

HRESULT CShellURLWrapper::GetParseResult(void)
{
	return m_actual->GetParseResult();
}

HRESULT CShellURLWrapper::SetUsnSource(ulong a1)
{
	return m_actual->SetRequestID(a1);
}

HRESULT CShellURLWrapper::GetUsnSource(ulong* a1)
{
	return m_actual->GetRequestID((int*)a1);
}

HRESULT CShellURLWrapper::SetNavFlags(int a1, int a2)
{
	return m_actual->SetNavFlags(a1, a2);
}

HRESULT CShellURLWrapper::GetCookie(ulong* a1)
{
	return m_actual->GetCookie(a1);
}

HRESULT CShellURLWrapper::Execute(void* a1, int* a2, ulong a3)
{
	return m_actual->Execute(a1, a2, a3);
}

HRESULT CShellURLWrapper::SetCurrentWorkingDir(LPITEMIDLIST a1)
{
	return m_actual->SetCurrentWorkingDir(a1);
}

HRESULT CShellURLWrapper::SetMessageBoxParent(HWND__* a1)
{
	return m_actual->SetMessageBoxParent(a1);
}

HRESULT CShellURLWrapper::GetPidlNoGenerate(LPITEMIDLIST* a1)
{
	return m_actual->GetPidlNoGenerate(a1);
}

HRESULT CShellURLWrapper::GetStandardParsingFlags(int a1)
{
	return m_actual->GetStandardParsingFlags(a1);
}
