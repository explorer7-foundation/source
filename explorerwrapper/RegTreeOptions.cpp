#include "RegTreeOptions.h"

CRegTreeOptionsWrapper::CRegTreeOptionsWrapper(IRegTreeOptions8* pUnk)
{
	m_cref = 1;
	regtree = pUnk;
}

CRegTreeOptionsWrapper::~CRegTreeOptionsWrapper()
{
	regtree->Release();
}

HRESULT __stdcall CRegTreeOptionsWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
	return regtree->QueryInterface(riid, ppvObject);
}

ULONG __stdcall CRegTreeOptionsWrapper::AddRef(void)
{
	regtree->AddRef();
	return InterlockedIncrement(&m_cref);
}

ULONG __stdcall CRegTreeOptionsWrapper::Release(void)
{
	regtree->Release();
	if (InterlockedDecrement(&m_cref) == 0)
	{
		free((void*)this);
		return 0;
	}
	return m_cref;
}

HRESULT __stdcall CRegTreeOptionsWrapper::InitTree(HWND hWnd, HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName)
{
	if (StrCmpA(lpSubKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel") == 0)
		return regtree->InitTree(hWnd, hKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel7", lpValueName);
	return regtree->InitTree(hWnd, hKey, lpSubKey, lpValueName);
}

HRESULT __stdcall CRegTreeOptionsWrapper::WalkTree(unsigned int command)
{
	return regtree->WalkTree(command);
}

HRESULT __stdcall CRegTreeOptionsWrapper::ToggleItem(_TREEITEM* pItem)
{
	return regtree->ToggleItem(pItem);
}

HRESULT __stdcall CRegTreeOptionsWrapper::ShowHelp(_TREEITEM* pItem, ULONG dwFlags)
{
	return regtree->ShowHelp(pItem, dwFlags);
}

HRESULT __stdcall CRegTreeOptionsWrapper::SetEventListener(IUnknown* pListener)
{
	return regtree->SetEventListener(pListener);
}
