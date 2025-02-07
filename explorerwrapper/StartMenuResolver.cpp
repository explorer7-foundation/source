#include "StartMenuResolver.h"
#include "StartMenuPin.h"
#include "dbgprint.h"
#include "PinnedList.h"
#pragma function(memset)

extern "C" HRESULT WINAPI Explorer_CoCreateInstance(
	__in   REFCLSID rclsid,
	__in   LPUNKNOWN pUnkOuter,
	__in   DWORD dwClsContext,
	__in   REFIID riid,
	__out  LPVOID* ppv
);

//constructor
CStartMenuResolver::CStartMenuResolver(IAppResolver8* newresolver)
{
	m_cRef = 0; //?
	m_resolver8 = newresolver;
	m_startmenuitemscache8 = nullptr;
	m_startmenuitemscache10 = nullptr;
}

CStartMenuResolver::CStartMenuResolver(IStartMenuItemsCache8 *newcache)
{
	m_cRef = 0;
	m_startmenuitemscache8 = newcache;
	m_startmenuitemscache10 = nullptr;
	CoCreateInstance(
		CLSID_StartMenuCacheAndAppResolver,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IAppResolver8,
		(LPVOID *)&m_resolver8
	);
}

CStartMenuResolver::CStartMenuResolver(IStartMenuItemsCache10 *newcache)
{
	m_cRef = 0;
	m_startmenuitemscache10 = newcache;
	m_startmenuitemscache8 = nullptr;
	CoCreateInstance(
		CLSID_StartMenuCacheAndAppResolver,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IAppResolver8,
		(LPVOID*)&m_resolver8
	);
}

CStartMenuResolver::~CStartMenuResolver()
{
	if (m_resolver8)
		m_resolver8->Release();

	if (m_startmenuitemscache8)
		m_startmenuitemscache8->Release();

	if (m_startmenuitemscache10)
		m_startmenuitemscache10->Release();
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IAppResolver7)
	{
		//dbgprintf(L"IID_IAppResolver7\n");
		*ppvObject = static_cast<IAppResolver7*>(this);
		AddRef();
		return S_OK;
	}
	if (riid == IID_IStartMenuItemsCache7)
	{
		dbgprintf(L"IID_IStartMenuItemsCache7\n");
		HRESULT ret = E_NOINTERFACE;
		if (m_startmenuitemscache8)
		{
			ret = m_startmenuitemscache8->QueryInterface(IID_IStartMenuItemsCache8, (PVOID *)&m_startmenuitemscache8);
			if (ret == S_OK)
			{
				dbgprintf(L"S_OK\n");
				*ppvObject = static_cast<IStartMenuItemsCache7 *>(this);
				AddRef();
			}
		}
		else if (m_startmenuitemscache10)
		{
			ret = m_startmenuitemscache10->QueryInterface(IID_IStartMenuItemsCache10, (PVOID*)&m_startmenuitemscache10);
			if (ret == S_OK)
			{
				dbgprintf(L"S_OK 2\n");
				*ppvObject = static_cast<IStartMenuItemsCache7 *>(this);
				AddRef();
			}
		}
		return ret;
	}
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CStartMenuResolver::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CStartMenuResolver::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

//IAppResolver7
HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetAppIDForShortcut(IShellItem* p1, LPWSTR* p2)
{
	dbgprintf(L"GetAppIDForShortcut");
	return m_resolver8->GetAppIDForShortcut(p1, p2);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetAppIDForWindow(HWND* p1, DWORD* p2, DWORD* p3, DWORD* p4, DWORD* p5)
{
	dbgprintf(L"GetAppIDForWindow");
	return m_resolver8->GetAppIDForWindow(p1, p2, p3, p4, p5);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetAppIDForProcess(ULONG_PTR p1, DWORD* p2, DWORD* p3, DWORD* p4, DWORD* p5)
{
	dbgprintf(L"GetAppIDForProcess");
	return m_resolver8->GetAppIDForProcess(p1, p2, p3, p4, p5);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetShortcutForProcess(ULONG_PTR p1, IUnknown* p2)
{
	dbgprintf(L"GetShortcutForProcess");
	return m_resolver8->GetShortcutForProcess(p1, p2);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetBestShortcutForAppID(DWORD* p1, IUnknown* p2)
{
	dbgprintf(L"GetBestShortcutForAppID");
	return m_resolver8->GetBestShortcutForAppID(p1, p2);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetBestShortcutAndAppIDForAppPath(DWORD* p1, IUnknown* p2, DWORD* p3)
{
	dbgprintf(L"GetBestShortcutAndAppIDForAppPath");
	return m_resolver8->GetBestShortcutAndAppIDForAppPath(p1, p2, p3);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::CanPinApp(IUnknown* p1)
{
	dbgprintf(L"CanPinApp");
	return m_resolver8->CanPinApp(p1);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetRelaunchProperties(HWND* p1, DWORD* p2, DWORD* p3, DWORD* p4, DWORD* p5, DWORD* p6)
{
	//dbgprintf(L"GetRelaunchProperties");
	return m_resolver8->GetRelaunchProperties(p1, p2, p3, p4, p5, p6, nullptr);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GenerateShortcutFromWindowProperties(HWND* p1, IUnknown* p2)
{
	dbgprintf(L"GenerateShortcutFromWindowProperties");
	return m_resolver8->GenerateShortcutFromWindowProperties(p1, p2);
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GenerateShortcutFromItemProperties(IUnknown* p1, IUnknown* p2)
{
	dbgprintf(L"GenerateShortcutFromItemProperties");
	return m_resolver8->GenerateShortcutFromItemProperties(p1, p2);
}

//IStartMenuItemsCache7
HRESULT STDMETHODCALLTYPE CStartMenuResolver::OnChangeNotify(unsigned int p1, long p2, PVOID* p3, PVOID* p4)
{
	HRESULT rslt;
	if (m_startmenuitemscache8)
		rslt = m_startmenuitemscache8->OnChangeNotify(p1, p2, p3, p4);
	else if (m_startmenuitemscache10)
		rslt = m_startmenuitemscache10->OnChangeNotify(p1, p2, p3, p4);
	dbgprintf(L"CStartMenuResolver::OnChangeNotify %p %p %p %p = %p", p1, p2, p3, p4, rslt);
	return rslt;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::PinListChanged(void)
{
	dbgprintf(L"CStartMenuResolver::PinListChanged");
	//we need to clear MFU cache, but we don't have one!
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetPinnedItemsCount(int* pCount)
{
	dbgprintf(L"GetPinnedItemsCount");
	*pCount = 0;
	IPinnedList2* pinList2 = 0;
	HRESULT rslt = Explorer_CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER, IID_IPinnedList2, (PVOID*)&pinList2);
	if (SUCCEEDED(rslt))
	{
		IEnumFullIDList* enumidlist;
		rslt = pinList2->EnumObjects(&enumidlist);
		if (SUCCEEDED(rslt))
		{
			LPITEMIDLIST pidl;
			ULONG wat;
			while (enumidlist->Next(1, &pidl, &wat) == S_OK)
			{
				(*pCount)++;
				CoTaskMemFree(pidl);
			}
			dbgprintf(L"CStartMenuResolver::GetPinnedItemsCount = %d", *pCount);
			enumidlist->Release();
		}
		pinList2->Release();
	}
	return rslt;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetStartMenuMFUList(unsigned int limit, IEnumStartMenuItem** penumStart, IEnumString** penumStrings, FILETIME* pNewFileTime)
{
	unsigned int cnt = 0;
	dbgprintf(L"CStartMenuResolver::GetStartMenuMFUList limit=%p filetime=%X_%X", limit, pNewFileTime->dwHighDateTime, pNewFileTime->dwLowDateTime);
	CEnumStartMenu* startenum = new CEnumStartMenu;
	*penumStart = (IEnumStartMenuItem*)startenum;
	*penumStrings = (IEnumString*)new CEnumStartMenu;
	//add pinned items
	IPinnedList2* startpinnedlist;
	IPinnedList2* taskbarpinnedlist;
	HRESULT rslt = Explorer_CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER, IID_IPinnedList2, (PVOID*)&startpinnedlist);
	if (FAILED(rslt)) return rslt;
	rslt = Explorer_CoCreateInstance(CLSID_TaskbarPin, NULL, CLSCTX_INPROC_SERVER, IID_IPinnedList2, (PVOID*)&taskbarpinnedlist);
	if (FAILED(rslt)) return rslt;
	IEnumFullIDList* enumidlist;
	rslt = startpinnedlist->EnumObjects(&enumidlist);
	if (SUCCEEDED(rslt))
	{
		STARTMENUITEM startitem = { 0 };
		while (enumidlist->Next(1, &startitem.pidlRelative, NULL) == S_OK)
		{
			IShellItem* shellitem;
			rslt = SHCreateItemFromIDList(startitem.pidlRelative, IID_IShellItem, (LPVOID*)&shellitem);
			if (SUCCEEDED(rslt))
			{
				rslt = m_resolver8->GetAppIDForShortcut(shellitem, &startitem.pszAppID);
				if (FAILED(rslt))
				{
					dbgprintf(L"GetAppIDForShortcut failed %p (shortcut broken?!)", rslt);
					rslt = shellitem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &startitem.pszAppID);
				}
				shellitem->Release();
				startitem.iPinPos = cnt;
				startenum->AddItem(&startitem);
				cnt++;
			}
		}
		enumidlist->Release();
	}
	//add separator
	STARTMENUITEM startitem = { 0 };
	startitem.iPinPos = -2;
	startenum->AddItem(&startitem);
	//add MFU items if needed
	if (limit > cnt)
	{
		//from start menu
		IStartMenuAppItems8* startitems;
		if (FAILED(m_resolver8->QueryInterface(IID_IStartMenuAppItems8, (LPVOID*)&startitems))) return S_FALSE;
		IObjectCollection* collection;
		if (FAILED(startitems->EnumItems(0, IID_IObjectCollection, (PVOID*)&collection))) return S_FALSE;
		UINT iLauncherCount = 0;
		UINT iLauncherItem;
		collection->GetCount(&iLauncherCount);
		for (iLauncherItem = 0; iLauncherItem < iLauncherCount; iLauncherItem++)
		{
			IPropertyStore* propstore;
			if (SUCCEEDED(collection->GetAt(iLauncherItem, IID_IPropertyStore, (PVOID*)&propstore)))
			{
				PROPVARIANT pvPidl;
				PROPVARIANT pvAppId;
				PROPVARIANT pvMetro;
				PROPVARIANT pvDual;
				propstore->GetValue(PKEY_AppUserModel_BestShortcut, &pvPidl);
				propstore->GetValue(PKEY_AppUserModel_ID, &pvAppId);
				propstore->GetValue(PKEY_AppUserModel_HostEnvironment, &pvMetro);
				propstore->GetValue(PKEY_AppUserModel_IsDualMode, &pvDual);
				//we're accepting only non-metro or dualmode shortcuts
				if (!pvMetro.intVal || pvDual.intVal)
				{
					STARTMENUITEM startitem = { 0 };
					if (SUCCEEDED(UAQueryShortcut((LPITEMIDLIST)pvPidl.caub.pElems, &startitem.ueminfo)) &&
						startitem.ueminfo.R && !startitem.ueminfo.fExcludeFromMFU)
						if (startpinnedlist->IsPinned((LPITEMIDLIST)pvPidl.caub.pElems) == S_FALSE) //IsPinned checks are VERY slow, at least under VMWare
							if (taskbarpinnedlist->IsPinned((LPITEMIDLIST)pvPidl.caub.pElems) == S_FALSE) //...why?!
							{
								startitem.pidlRelative = ILClone((LPITEMIDLIST)pvPidl.caub.pElems);
								startitem.pszAppID = CoAllocString(pvAppId.bstrVal);
								startitem.iPinPos = -1;
								startenum->AddItem(&startitem);
							}
				}
				propstore->Release();
			}
		}
		collection->Release();
		startitems->Release();
		//from desktop
		LPITEMIDLIST pidlitem;
		IShellFolder* dsf;
		IEnumIDList* enumdesktop;
		SHGetDesktopFolder(&dsf);
		dsf->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_FASTITEMS, &enumdesktop);

		while (enumdesktop->Next(1, &pidlitem, NULL) == S_OK)
		{
			SFGAOF attrs = SFGAO_LINK;
			dsf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlitem, &attrs);

			if (attrs & SFGAO_LINK)
			{
				STARTMENUITEM startitem = { 0 };
				SHGetRealIDL(dsf, pidlitem, &startitem.pidlRelative);

				if (SUCCEEDED(UAQueryShortcut(startitem.pidlRelative, &startitem.ueminfo)) &&
					startitem.ueminfo.R && !startitem.ueminfo.fExcludeFromMFU)
					if (taskbarpinnedlist->IsPinned(startitem.pidlRelative) == S_FALSE)
						if (startpinnedlist->IsPinned(startitem.pidlRelative) == S_FALSE)
						{
							IShellItem* shellitem;
							rslt = SHCreateItemFromIDList(startitem.pidlRelative, IID_IShellItem, (LPVOID*)&shellitem);
							if (SUCCEEDED(rslt))
							{
								rslt = m_resolver8->GetAppIDForShortcut(shellitem, &startitem.pszAppID);
								if (FAILED(rslt))
								{
									dbgprintf(L"GetAppIDForShortcut failed %p (shortcut broken?!)", rslt);
									rslt = shellitem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &startitem.pszAppID);
								}
								shellitem->Release();
								startitem.iPinPos = -1;
								startenum->AddItem(&startitem);
							}
						}
			}

			ILFree(pidlitem);
		}
		enumdesktop->Release();
		dsf->Release();
	}
	startpinnedlist->Release();
	taskbarpinnedlist->Release();
	startenum->Sort();
	startenum->SetLimit(limit);
	startenum->RemoveDuplicates();
	GetSystemTimeAsFileTime(pNewFileTime);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::RegisterSMNotify(IUnknown* p1)
{
	dbgprintf(L"RegisterSMNotify");
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::RegisterARNotify(IUnknown* p1)
{
	dbgprintf(L"RegisterARNotify");
	if (m_startmenuitemscache8)
		return m_startmenuitemscache8->RegisterARNotify(new CAppResolverNotify8((IAppResolverNotify7*)p1));
	else if (m_startmenuitemscache10)
		return m_startmenuitemscache10->RegisterARNotify(new CAppResolverNotify8((IAppResolverNotify7*)p1));

	return E_ABORT;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::SetAltName(PVOID* p1, DWORD* p2, PVOID* p3)
{
	dbgprintf(L"CStartMenuResolver::SetAltName %p %p %p", p1, p2, p3);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CStartMenuResolver::GetAltName(PVOID* p1, DWORD* p2)
{
	dbgprintf(L"CStartMenuResolver::GetAltName %p %p", p1, p2);
	return E_NOTIMPL;
}
