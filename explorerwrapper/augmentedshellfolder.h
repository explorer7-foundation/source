#pragma once
#define INITGUID
#include "framework.h"

MIDL_INTERFACE("2f711b17-773c-41d4-93fa-7f23edcecb66")
IAugmentedShellFolder : public IShellFolder
{
    STDMETHOD(AddNameSpace)(LPGUID, IShellFolder*, LPCITEMIDLIST, ULONG, ULONG) PURE;
    STDMETHOD(GetNameSpaceID)(LPCITEMIDLIST, LPGUID) PURE;
    STDMETHOD(QueryNameSpace)(ULONG, LPGUID, IShellFolder**) PURE;
    STDMETHOD(EnumNameSpace)(ULONG, PULONG) PURE;
};

MIDL_INTERFACE("8DB3B3F4-6CFE-11d1-8AE9-00C04FD918D0")
IAugmentedShellFolder2 : public IAugmentedShellFolder
{
     STDMETHOD(UnWrapIDList)(LPCITEMIDLIST, LONG, IShellFolder**, LPITEMIDLIST*, LPITEMIDLIST*, LONG*) PURE;
};