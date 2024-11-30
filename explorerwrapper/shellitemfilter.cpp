#include "shellitemfilter.h"
#include "augmentedshellfolder.h"
#include "Shlobj_core.h"
#include "propkey.h"
#include "startmenuresolver.h"

//0000000180751608		SHGetFolderPathEx	api-ms-win-storage-exports-internal-l1-1-0
CStartMenuItemFilter::CStartMenuItemFilter()
{
    static HRESULT(__fastcall * fSHGetFolderPathEx)(
        _In_     REFKNOWNFOLDERID rfid,
        _In_     DWORD            dwFlags,
        _In_opt_ HANDLE           hToken,
        _Out_    LPWSTR           pszPath,
        _In_     UINT             cchPath
        ) = (decltype(fSHGetFolderPathEx))(GetProcAddress(GetModuleHandleW(L"api-ms-win-storage-exports-internal-l1-1-0"),"SHGetFolderPathEx"));
    
    m_ref = 0;
    
    memset(m_programs,0,sizeof(m_programs));
    memset(m_commonPrograms,0,sizeof(m_commonPrograms));
    memset(m_adminTools,0,sizeof(m_adminTools));
    memset(m_commonAdminTools,0,sizeof(m_commonAdminTools));
    memset(m_games,0,sizeof(m_games));
    
    WCHAR psz[264];
    fSHGetFolderPathEx(FOLDERID_Programs,0,0,psz,260);
    lstrcpy(LPWSTR(this->m_programs),psz);
    fSHGetFolderPathEx(FOLDERID_CommonPrograms,0,0,psz,260);
    lstrcpy(LPWSTR(this->m_commonPrograms),psz);
    fSHGetFolderPathEx(FOLDERID_CommonAdminTools, 0, 0, psz, 260);
    lstrcpy(LPWSTR(this->m_commonAdminTools), psz);
    fSHGetFolderPathEx(FOLDERID_AdminTools, 0, 0, psz, 260);
    lstrcpy(LPWSTR(this->m_adminTools), psz);
    fSHGetFolderPathEx(FOLDERID_Games, 0, 0, psz, 260);
    lstrcpy(LPWSTR(this->m_games), psz);
}

HRESULT __stdcall CStartMenuItemFilter::QueryInterface(REFIID riid, void** ppvObject)
{
    *ppvObject = 0;

    IUnknown* ppv = nullptr;
    if (riid == IID_IUnknown)
    {
        ppv = static_cast<IUnknown*>(this);
    }
    else if (riid == IID_IShellItemFilter)
    {
        ppv = static_cast<IShellItemFilter*>(this);
    }
    else if (riid == BHID_SFObject)
    {
        *ppvObject = 0;
        return E_NOTIMPL;
    }

    if (ppv)
    {
        ppv->AddRef();
        *ppvObject = ppv;
        return S_OK;
    }

    return E_NOTIMPL;
}

ULONG __stdcall CStartMenuItemFilter::AddRef(void)
{
    return InterlockedIncrement(&this->m_ref);
}

ULONG __stdcall CStartMenuItemFilter::Release(void)
{
    auto ref = _InterlockedDecrement(&this->m_ref);
    if (!ref)
        delete this;
    return ref;
}

bool IsMergedFolderGUID(IShellFolder* ShellFolder, LPCITEMIDLIST pidl, REFGUID Guid)
{
    IAugmentedShellFolder* agShellFolder;
    bool bMatches = false;
    if (SUCCEEDED(ShellFolder->QueryInterface(IID_PPV_ARGS(&agShellFolder))))
    {
        GUID cmp;
        if (SUCCEEDED(agShellFolder->GetNameSpaceID(pidl, &cmp)))
        {
            bMatches = cmp == Guid;
        }
        agShellFolder->Release();
    }
    return bMatches;
}

HRESULT __stdcall CStartMenuItemFilter::IncludeItem(IShellItem* psi)
{
    ULONG v11;
    HRESULT v4;

    IParentAndItem* ParentAndItem;
    v4 = psi->QueryInterface(IID_PPV_ARGS(&ParentAndItem));
    if (v4 >= 0)
    {
        IShellFolder* v7;
        LPCITEMIDLIST pv;
        v4 = ParentAndItem->GetParentAndItem(0, &v7, (LPITEMIDLIST*)&pv);
        if (v4 >= 0)
        {
            if (IsMergedFolderGUID(v7, pv, FOLDERID_AppsFolder))
            {
                IShellItem2* v10;
                v4 = psi->QueryInterface(IID_PPV_ARGS(&v10));
                if (v4 >= 0 && v10->GetUInt32(PKEY_AppUserModel_HostEnvironment, &v11) >= 0 && v11 != 1)
                {
                    v4 = 1;
                }
                v10->Release();
            }
            else if (!FilterPidl(v7, pv))
            {
                v4 = 1;
            }
            v7->Release();
            CoTaskMemFree((LPVOID)pv);
        }
        ParentAndItem->Release();
    }

    return v4;
}

HRESULT __stdcall CStartMenuItemFilter::GetEnumFlagsForItem(IShellItem* psi, SHCONTF* pgrfFlags)
{
    return E_NOTIMPL;
}

bool CStartMenuItemFilter::FilterPidl(IShellFolder* shellfolder, LPCITEMIDLIST idl)
{
    bool v4 = 1;
    STRRET strret;
    if (shellfolder->GetDisplayNameOf(idl, 0x8000u, &strret) >= 0)
    {
        //idk what this is meant to do, so its commented out for now, in the future, it might be a good idea to see what it does and add it, just in case its needed
        /*
        v5 = *(_QWORD *)(this + 72);
        if ( v5 )
        {
            if ( !(*(unsigned int (__fastcall **)(_QWORD, WCHAR *))(*(_QWORD *)v5 + 24LL))(*(_QWORD *)(this + 72), String1) )
            return 0;
        }
        */
        if (this->m_programs && !lstrcmpiW(strret.pOleStr, this->m_programs))
            return 0;
        
        if (this->m_commonPrograms && !lstrcmpiW(strret.pOleStr, this->m_commonPrograms))
            return 0;
        
        if (this->m_commonAdminTools && !lstrcmpiW(strret.pOleStr, this->m_commonAdminTools))
            return 0;
        
        if (this->m_adminTools && !lstrcmpiW(strret.pOleStr, this->m_adminTools))
            return 0;

        if (this->m_games && !lstrcmpiW(strret.pOleStr, this->m_games))
            return 0;
    }
    return v4;
}