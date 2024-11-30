#pragma once
#define INITGUID
#include "framework.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_AutoPlayUI,0x83b52078, 0xE93E, 0x425B, 0x92,0x6F,0xDE,0x61,0x69,0x87,0x5E,0x41); //83b52078_e93e_425b_926f_de6169875e41
DEFINE_GUID(IID_AutoPlayUI,0x9394e091, 0xa034, 0x4f1b, 0xb0,0x88,0x5b,0x53,0xa0,0x6c,0x65,0xfa); //9394e091_a034_4f1b_b088_5b53a06c65fa
#pragma endregion

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IAutoPlayUI: public IUnknown
{
public:
	STDMETHOD(InitVolumeAutoplay)(IUnknown *,LPCWSTR,LPCWSTR,ULONG,ULONG,ULONG,LPCWSTR,LPCWSTR,int,LPCWSTR,LPCWSTR,HWND) PURE;
	STDMETHOD(InitNoContentAutoplay)(IUnknown *,REFGUID,LPCWSTR,ULONG,int,LPCWSTR,LPCWSTR,LPCWSTR) PURE;
	STDMETHOD(InitDirectAutoPlay)(IUnknown *,LPCWSTR,HWND) PURE;
	STDMETHOD(ToastPromptForChkDsk)(LPCWSTR,int *,int *) PURE;
	STDMETHOD(LaunchDeviceHandler)(LPCWSTR,LPCWSTR,LPCWSTR) PURE;
	STDMETHOD(IsDialogClosed)(void) PURE;
	STDMETHOD(SniffComplete)(ULONG) PURE;
	STDMETHOD(CloseDialog)(void) PURE;
	STDMETHOD(AddContentType)(ULONG) PURE;
	STDMETHOD(MoreInterfaceArrived)(LPCWSTR) PURE;
	STDMETHOD(SetChkDskCompleted)(void) PURE;
};

class CAutoPlayWrapper: public IAutoPlayUI
{
public:
	CAutoPlayWrapper(IAutoPlayUI *autoui);
	~CAutoPlayWrapper();

	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
    STDMETHODIMP_(ULONG) AddRef(void);    
	STDMETHODIMP_(ULONG) Release(void);

	//IAutoPlayUI
	STDMETHODIMP InitVolumeAutoplay(IUnknown *,LPCWSTR,LPCWSTR,ULONG,ULONG,ULONG,LPCWSTR,LPCWSTR,int,LPCWSTR,LPCWSTR,HWND);
	STDMETHODIMP InitNoContentAutoplay(IUnknown *,REFGUID,LPCWSTR,ULONG,int,LPCWSTR,LPCWSTR,LPCWSTR);
	STDMETHODIMP InitDirectAutoPlay(IUnknown *,LPCWSTR,HWND);
	STDMETHODIMP ToastPromptForChkDsk(LPCWSTR,int *,int *);
	STDMETHODIMP LaunchDeviceHandler(LPCWSTR,LPCWSTR,LPCWSTR);
	STDMETHODIMP IsDialogClosed(void);
	STDMETHODIMP SniffComplete(ULONG);
	STDMETHODIMP CloseDialog(void);
	STDMETHODIMP AddContentType(ULONG);
	STDMETHODIMP MoreInterfaceArrived(LPCWSTR);
	STDMETHODIMP SetChkDskCompleted(void);
private:
	IAutoPlayUI *m_autoui;
	long m_cRef;
};

HRESULT WINAPI Shell32_CoCreateInstance(
  __in   REFCLSID rclsid,
  __in   LPUNKNOWN pUnkOuter,
  __in   DWORD dwClsContext,
  __in   REFIID riid,
  __out  LPVOID *ppv
);
