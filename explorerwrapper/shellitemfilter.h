#pragma once
#include "framework.h"

class CStartMenuItemFilter : public IShellItemFilter
{
public:
    CStartMenuItemFilter();

    //IUnknown interface
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    //IShellItemFilter interface
    STDMETHOD(IncludeItem)(IShellItem* psi);
    STDMETHOD(GetEnumFlagsForItem)(IShellItem* psi,SHCONTF* pgrfFlags);

    bool FilterPidl(IShellFolder* shellfolder, LPCITEMIDLIST idl);

    ULONG m_ref;
    WCHAR m_programs[MAX_PATH];
    WCHAR m_commonPrograms[MAX_PATH];
    WCHAR m_adminTools[MAX_PATH];
    WCHAR m_commonAdminTools[MAX_PATH];
    WCHAR m_games[MAX_PATH];
};