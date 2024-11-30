/*
#include "pshpack1.h"
typedef struct
{
	BYTE        jmp_func; 
	DWORD       relay_offset;
} THUNKPROC, *PTHUNKPROC;
#include "poppack.h"

typedef HRESULT (WINAPI *RoActivateInstanceAPI)(LPWSTR,PVOID**);
typedef LPWSTR (WINAPI* WindowsGetStringRawBufferAPI)(LPWSTR,int*);
WindowsGetStringRawBufferAPI WindowsGetStringRawBuffer;

RoActivateInstanceAPI RoActivateInstance;
char RoActivateInstanceOrig[5];
HRESULT WINAPI RoActivateInstanceNEW(LPWSTR activatableClassId, PVOID **instance)
{
	dbgprintf(L"RoActivateInstance %s %p",WindowsGetStringRawBuffer(activatableClassId,NULL),instance);
	memcpy(RoActivateInstance,RoActivateInstanceOrig,5);
	FlushInstructionCache(GetCurrentProcess,RoActivateInstance,5);
	HRESULT ret = RoActivateInstance(activatableClassId,instance);	
	dbgprintf(L"RoActivateInstance %s %p = %p",activatableClassId,instance,ret);
	PTHUNKPROC patch = (PTHUNKPROC)RoActivateInstance;
	patch->jmp_func = 0xE9;
	patch->relay_offset = (DWORD)RoActivateInstanceNEW - (DWORD)&patch->relay_offset - 4;
	FlushInstructionCache(GetCurrentProcess,RoActivateInstance,5);
	return ret;	
}*/

	/*#ifndef _WIN64	
	//hook roactivateinstance
	DWORD wat;
	WindowsGetStringRawBuffer = (WindowsGetStringRawBufferAPI)GetProcAddress(LoadLibrary(L"combase.dll"),"WindowsGetStringRawBuffer");
	RoActivateInstance = (RoActivateInstanceAPI)GetProcAddress(LoadLibrary(L"combase.dll"),"RoActivateInstance");
	VirtualProtect(RoActivateInstance,5,PAGE_EXECUTE_READWRITE,&wat);
	dbgprintf(L"RoActivateInstance %p",RoActivateInstance);
	PTHUNKPROC patch = (PTHUNKPROC)RoActivateInstance;
	memcpy(RoActivateInstanceOrig,RoActivateInstance,5);
	patch->jmp_func = 0xE9;
	patch->relay_offset = (DWORD)RoActivateInstanceNEW - (DWORD)&patch->relay_offset - 4;		
	#endif*/

/*SHOneParamAPI DwmpBeginTransitionRequest;
HRESULT WINAPI DwmpBeginTransitionRequestNEW(DWORD sth)
{
	dbgprintf(L"DwmpBeginTransitionRequest",sth);
	HRESULT ret = DwmpBeginTransitionRequest(sth);
	dbgprintf(L"DwmpBeginTransitionRequest %p = %p",sth,ret);
	return ret;
}*/

/*
UINT_PTR WINAPI SetTimer_WUI( HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc )
{
	DWORD vebx;
	__asm { mov vebx, ebx }
	if ( nIDEvent == 0x2252CE37 )
	{
		ShowWindow(hWnd,SW_HIDE);
		DWORD* wat = (DWORD*)(vebx + 0x34);
		wat = (DWORD*)*wat;
		wat++;
		dbgprintf(L"wat4 is %p",*wat);
		*wat = 1;
		wat++;
		dbgprintf(L"wat8 is %p",*wat);
		*wat = 0;
	}
	return SetTimer(hWnd,nIDEvent,uElapse,lpTimerFunc);
}

*/

	/*DwmpBeginTransitionRequest = (SHOneParamAPI)GetProcAddress(GetModuleHandle(L"dwmapi.dll"),(LPSTR)138);
	DWORD* wat = (DWORD*)((DWORD)immersiveui + 0x70378);
	DWORD wut;
	VirtualProtect(wat,4,PAGE_EXECUTE_READWRITE,&wut);
	*wat = (DWORD)DwmpBeginTransitionRequestNEW;*/

/*
extern "C" HRESULT WINAPI CoCreateInstanceSPY(
  __in   REFCLSID rclsid,
  __in   LPUNKNOWN pUnkOuter,
  __in   DWORD dwClsContext,
  __in   REFIID riid,
  __out  LPVOID *ppv
)
{
	HRESULT result;
	#ifdef COMDEBUG
	WCHAR clsid[40];
	WCHAR iid[40];
	WCHAR regpath[100];
	WCHAR dllname[100];
	HKEY key;
	HKEY key2;	
	LONG sz = 100;
	StringFromGUID2(rclsid,clsid,40);
	StringFromGUID2(riid,iid,40);
	lstrcpy(regpath,L"CLSID\\");	
	lstrcat(regpath,clsid);
	RegOpenKey(HKEY_CLASSES_ROOT,regpath,&key);
	lstrcat(regpath,L"\\InProcServer32");
	RegOpenKey(HKEY_CLASSES_ROOT,regpath,&key2);
	RegQueryValue(key,NULL,regpath,&sz);
	sz = 100;
	RegQueryValue(key2,NULL,dllname,&sz);
	RegCloseKey(key);
	RegCloseKey(key2);
	#endif
	if (rclsid == CLSID_SysTray) //create Metro before tray
	{
		HookImmersive();
		//CreateTwinUI();
	}
	if (rclsid == CLSID_RegTreeOptions && riid == IID_IRegTreeOptions7) //upgrading RegTreeOptions interface
	{

		result = CoCreateInstance(rclsid,pUnkOuter,dwClsContext,IID_IRegTreeOptions8,ppv);	
	}
	else
		result = CoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);

	if (rclsid == CLSID_StartMenuCacheAndAppResolver && result == E_NOINTERFACE)
	{
		PVOID rslvr8 = NULL;
		CoCreateInstance(rclsid,pUnkOuter,dwClsContext,IID_IAppResolver8,&rslvr8);
		//create our object
		CStartMenuResolver* resolver7 = new CStartMenuResolver((IAppResolver8*)rslvr8);
		result = resolver7->QueryInterface(riid,ppv);
	}
	if (result == S_OK && rclsid == CLSID_SysTray) //wrap stobject
	{
		*ppv = new CSysTrayWrapper((IOleCommandTarget*)*ppv);
	}

	if (result == S_OK && rclsid == CLSID_AuthUIShutdownChoices) //wrap authui
	{
		*ppv = new CAuthUIWrapper((IShutdownChoices*)*ppv);
	}

	#ifdef COMDEBUG
	if (result == S_OK)
		dbgprintf(L"OKAY: %8X %s [%s] = %8X (%s) %s\n",_ReturnAddress(),clsid,iid,*ppv,regpath,dllname);
	else
		dbgprintf(L"FAIL: %8X %s [%s] = %8X (%s) %s\n",_ReturnAddress(),clsid,iid,result,regpath,dllname);
	#endif
	return result;
}
*/