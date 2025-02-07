#pragma once
#define INITGUID
#include "common.h"
#include "appresolvernotify.h"
#include "EnumStartMenu.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_StartMenuCacheAndAppResolver, 0x660B90C8, 0x73A9, 0x4B58, 0x8C, 0xAE, 0x35, 0x5B, 0x7F, 0x55, 0x34, 0x1B);
DEFINE_GUID(IID_IAppResolver7, 0x46a6eeff, 0x908e, 0x4dc6, 0x92, 0xA6, 0x64, 0xbe, 0x91, 0x77, 0xb4, 0x1c); //46a6eeff_908e_4dc6_92a6_64be9177b41c
DEFINE_GUID(IID_IAppResolver8, 0xde25675a, 0x72de, 0x44b4, 0x93, 0x73, 0x05, 0x17, 0x04, 0x50, 0xc1, 0x40); //de25675a_72de_44b4_9373_05170450c140

DEFINE_GUID(IID_IStartMenuItemsCache7, 0x05a232fd, 0x2bfb, 0x4349, 0x9d, 0x48, 0x47, 0x87, 0xf3, 0x17, 0xf5, 0x0a); //05a232fd_2bfb_4349_9d48_4787f317f50a
DEFINE_GUID(IID_IStartMenuItemsCache8, 0x934332DD, 0x0B0FE, 0x41F9, 0x0BC, 0x63, 0x9C, 0x7F, 0x9F, 0x3C, 0x3A, 0x0EC); //_GUID_934332dd_b0fe_41f9_bc63_9c7f9f3c3aec
DEFINE_GUID(IID_IStartMenuItemsCache10, 0x0BA5A92AE, 0x0BFD7, 0x4916, 0x85, 0x4F, 0x6B, 0x3A, 0x40, 0x2B, 0x84, 0x0A8); //_GUID_ba5a92ae_bfd7_4916_854f_6b3a402b84a8

DEFINE_GUID(IID_IStartMenuAppItems8, 0x2C5CCF3, 0x805F, 0x4654, 0x0A7, 0x0B7, 0x34, 0x0A, 0x74, 0x33, 0x53, 0x65); //02c5ccf3_805f_4654_a7b7_340a74335365

DEFINE_PROPERTYKEY(PKEY_AppUserModel_BestShortcut, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 10);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_HostEnvironment, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 14);
//DEFINE_PROPERTYKEY(PKEY_AppUserModel_IsDualMode, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 11);
#pragma endregion

MIDL_INTERFACE("46a6eeff-908e-4dc6-92a6-64be9177b41c")
IAppResolver7: public IUnknown
{
public:
	STDMETHOD(GetAppIDForShortcut)(IShellItem*, LPWSTR*) PURE;
	STDMETHOD(GetAppIDForWindow)(HWND*,DWORD*,DWORD*,DWORD*,DWORD*) PURE;
	STDMETHOD(GetAppIDForProcess)(ULONG_PTR,DWORD*,DWORD*,DWORD*,DWORD*) PURE;
	STDMETHOD(GetShortcutForProcess)(ULONG_PTR,IUnknown*) PURE;
	STDMETHOD(GetBestShortcutForAppID)(DWORD*,IUnknown*) PURE;
	STDMETHOD(GetBestShortcutAndAppIDForAppPath)(DWORD*,IUnknown*,DWORD*) PURE;
	STDMETHOD(CanPinApp)(IUnknown*) PURE;
	STDMETHOD(GetRelaunchProperties)(HWND*,DWORD*,DWORD*,DWORD*,DWORD*,DWORD*) PURE;
	STDMETHOD(GenerateShortcutFromWindowProperties)(HWND*,IUnknown*) PURE;
	STDMETHOD(GenerateShortcutFromItemProperties)(IUnknown*,IUnknown*) PURE;
};

MIDL_INTERFACE("de25675a-72de-44b4-9373-05170450c140")
IAppResolver8: public IUnknown
{
public:
	STDMETHOD(GetAppIDForShortcut)(IShellItem*, LPWSTR*) PURE;
	STDMETHOD(GetAppIDForShortcutObject)(IUnknown*, IUnknown*, DWORD*) PURE;
	STDMETHOD(GetAppIDForWindow)(HWND*,DWORD*,DWORD*,DWORD*,DWORD*) PURE;
	STDMETHOD(GetAppIDForProcess)(ULONG_PTR,DWORD*,DWORD*,DWORD*,DWORD*) PURE;
	STDMETHOD(GetShortcutForProcess)(ULONG_PTR,IUnknown*) PURE;
	STDMETHOD(GetBestShortcutForAppID)(DWORD*,IUnknown*) PURE;
	STDMETHOD(GetBestShortcutAndAppIDForAppPath)(DWORD*,IUnknown*,DWORD*) PURE;
	STDMETHOD(CanPinApp)(IUnknown*) PURE;
	STDMETHOD(CanPinAppShortcut)(IUnknown*, IUnknown*) PURE;
	STDMETHOD(GetRelaunchProperties)(HWND*,DWORD*,DWORD*,DWORD*,DWORD*,DWORD*, int* a7) PURE;
	STDMETHOD(GenerateShortcutFromWindowProperties)(HWND*,IUnknown*) PURE;
	STDMETHOD(GenerateShortcutFromItemProperties)(IUnknown*,IUnknown*) PURE;
};

MIDL_INTERFACE("05a232fd-2bfb-4349-9d48-4787f317f50a")
IStartMenuItemsCache7: public IUnknown
{
public:
	STDMETHOD(OnChangeNotify)(unsigned int,long,PVOID*,PVOID*) PURE;
	STDMETHOD(PinListChanged)(void) PURE;
	STDMETHOD(GetPinnedItemsCount)(int*) PURE;
	STDMETHOD(GetStartMenuMFUList)(unsigned int,IEnumStartMenuItem**,IEnumString**,FILETIME*) PURE;
	STDMETHOD(RegisterSMNotify)(IUnknown*) PURE;
	STDMETHOD(RegisterARNotify)(IUnknown*) PURE;
	STDMETHOD(SetAltName)(PVOID*,DWORD*,PVOID*) PURE;
	STDMETHOD(GetAltName)(PVOID*,DWORD*) PURE;
};

//MIDL_INTERFACE("bb9786b2-efe6-4f1e-a3bd-67f97d0085bf")
MIDL_INTERFACE("934332dd-b0fe-41f9-bc63-9c7f9f3c3aec")
IStartMenuItemsCache8: public IUnknown
{
public:
	STDMETHOD(OnChangeNotify)(unsigned int,long,PVOID*,PVOID*) PURE;
	STDMETHOD(RegisterForNotifications)(void*) PURE;
	STDMETHOD(UnregisterForNotifications)(void) PURE;
	STDMETHOD(PauseNotifications)(void) PURE;
	STDMETHOD(ResumeNotifications)(void) PURE;
	STDMETHOD(RegisterARNotify)(IUnknown*) PURE;
	STDMETHOD(RefreshCache)(int) PURE;
	STDMETHOD(ReleaseGlobalCacheObject)(void) PURE;
	STDMETHOD(IsCacheMatchingLanguage)(int*) PURE;
};
MIDL_INTERFACE("ba5a92ae-bfd7-4916-854f-6b3a402b84a8")
IStartMenuItemsCache10: public IUnknown
{
public:
	STDMETHOD(OnChangeNotify)(unsigned int,long,PVOID*,PVOID*) PURE;
	STDMETHOD(RegisterForNotifications)(void*) PURE;
	STDMETHOD(UnregisterForNotifications)(void) PURE;
	STDMETHOD(PauseNotifications)(void) PURE;
	STDMETHOD(ResumeNotifications)(void) PURE;
	STDMETHOD(RegisterARNotify)(IUnknown*) PURE;
	STDMETHOD(RefreshCache)(int) PURE;
	STDMETHOD(ReleaseGlobalCacheObject)(void) PURE;
	STDMETHOD(IsCacheMatchingLanguage)(int*) PURE;
	STDMETHOD(EnableAppUsageData)(void) PURE;
};

/*
CExtractConstIcon::AddRef(void)
CAppResolver::Release(void)
CAppResolver::OnChangeNotify(uint,long,_ITEMIDLIST_ABSOLUTE const *,_ITEMIDLIST_ABSOLUTE const *)
CAppResolver::RegisterForNotifications(IAppResolverProxy *)
CAppResolver::UnregisterForNotifications(void)
CAppResolver::PauseNotifications(void)
CAppResolver::ResumeNotifications(void)
CAppResolver::RegisterARNotify(IAppResolverNotify *)
CAppResolver::RefreshCache(START_MENU_REFRESH_CACHE_FLAGS)
CAppResolver::ReleaseGlobalCacheObject(void)
CAppResolver::IsCacheMatchingLanguage(int *)
*/

/*
?AddRef@CCommonParentUndoUnit@@WBA@EAAKXZ ; [thunk]:CCommonParentUndoUnit::AddRef`adjustor{16}' (void)
?Release@CAppResolver@@WBA@EAAKXZ ; [thunk]:CAppResolver::Release`adjustor{16}' (void)
?EnumItems@CAppResolver@@UEAAJW4START_MENU_APP_ITEMS_FLAGS@@AEBU_GUID@@PEAPEAX@Z ; CAppResolver::EnumItems(START_MENU_APP_ITEMS_FLAGS,_GUID const &,void * *)
?GetItem@CAppResolver@@UEAAJW4START_MENU_APP_ITEMS_FLAGS@@PEBGAEBU_GUID@@PEAPEAX@Z ; CAppResolver::GetItem(START_MENU_APP_ITEMS_FLAGS,ushort const *,_GUID const &,void * *)
?GetItemByAppPath@CAppResolver@@UEAAJPEBGAEBU_GUID@@PEAPEAX@Z ; CAppResolver::GetItemByAppPath(ushort const *,_GUID const &,void * *)
*/

//MIDL_INTERFACE("33f71155-c2e9-4ffe-9786-a32d98577cff")
MIDL_INTERFACE("02c5ccf3-805f-4654-a7b7-340a74335365")
IStartMenuAppItems8: public IUnknown
{
public:
	STDMETHOD(EnumItems)(int, REFIID, PVOID*) PURE;
	STDMETHOD(GetItem)(int, LPWSTR, const IID& riid, PVOID*) PURE;
	STDMETHOD(GetItemByAppPath)(const WCHAR*, _GUID const&, void**) PURE;
};

class CStartMenuResolver : public IAppResolver7, IStartMenuItemsCache7
{
public:
	CStartMenuResolver(IAppResolver8* newresolver);
	CStartMenuResolver(IStartMenuItemsCache8 *newcache);
	CStartMenuResolver(IStartMenuItemsCache10 *newcache);
	~CStartMenuResolver();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG)  Release(void);

	//IAppResolver7
	STDMETHODIMP GetAppIDForShortcut(IShellItem*, LPWSTR*);
	STDMETHODIMP GetAppIDForWindow(HWND*, DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GetAppIDForProcess(ULONG_PTR, DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GetShortcutForProcess(ULONG_PTR, IUnknown*);
	STDMETHODIMP GetBestShortcutForAppID(DWORD*, IUnknown*);
	STDMETHODIMP GetBestShortcutAndAppIDForAppPath(DWORD*, IUnknown*, DWORD*);
	STDMETHODIMP CanPinApp(IUnknown*);
	STDMETHODIMP GetRelaunchProperties(HWND*, DWORD*, DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GenerateShortcutFromWindowProperties(HWND*, IUnknown*);
	STDMETHODIMP GenerateShortcutFromItemProperties(IUnknown*, IUnknown*);

	//IStartMenuItemsCache7
	STDMETHODIMP OnChangeNotify(unsigned int, long, PVOID*, PVOID*);
	STDMETHODIMP PinListChanged(void);
	STDMETHODIMP GetPinnedItemsCount(int*);
	STDMETHODIMP GetStartMenuMFUList(unsigned int, IEnumStartMenuItem**, IEnumString**, FILETIME*);
	STDMETHODIMP RegisterSMNotify(IUnknown*);
	STDMETHODIMP RegisterARNotify(IUnknown*);
	STDMETHODIMP SetAltName(PVOID*, DWORD*, PVOID*);
	STDMETHODIMP GetAltName(PVOID*, DWORD*);
private:
	IAppResolver8* m_resolver8;
	IStartMenuItemsCache8* m_startmenuitemscache8;
	IStartMenuItemsCache10* m_startmenuitemscache10;
	long m_cRef;
};
