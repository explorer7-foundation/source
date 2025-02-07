#pragma once
#define INITGUID
#include "common.h"

#pragma region GUID definitions
DEFINE_GUID(CLSID_ImmersiveShellBuilder,0xc71c41f1, 0xddad, 0x42dc, 0xa8,0xfc,0xf5,0xbf,0xc6,0x1d,0xf9,0x57); //c71c41f1_ddad_42dc_a8fc_f5bfc61df957
DEFINE_GUID(IID_ImmersiveShellBuilder,0x1c56b3e4, 0xe6ea, 0x4ced, 0x8a,0x74,0x73,0xb7,0x2c,0x6b,0xd4,0x35); //1c56b3e4_e6ea_4ced_8a74_73b72c6bd435

DEFINE_GUID(IID_ImmersiveBehavior,0x139275e0, 0xd644, 0x4214, 0xb4,0x5e,0xd9,0x27,0x8c,0x4a,0x85,0x01); //139275e0_d644_4214_b45e_d9278c4a8501

DEFINE_GUID(CLSID_NowPlayingSessionManager, 0xbcbb9860, 0xc012, 0x4ad7, 0xa9, 0x38, 0x6e, 0x33, 0x7a, 0xe6, 0xab, 0xa5);
#pragma endregion

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveBehavior: public IUnknown
{
public:
	STDMETHOD(OnImmersiveThreadStart)(void) PURE;
	STDMETHOD(OnImmersiveThreadStop)(void) PURE;
	STDMETHOD(GetMaximumComponentCount)(unsigned int *count) PURE;
	STDMETHOD(CreateComponent)(unsigned int number, IUnknown** component) PURE;
	STDMETHOD(ShouldCreateComponent)(unsigned int number, int* allowed) PURE;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveShellController: public IUnknown
{
public:
	STDMETHOD(Start)(void) PURE;
	STDMETHOD(Stop)(void) PURE;
	STDMETHOD(SetCreationBehavior)(IImmersiveBehavior*) PURE;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IImmersiveShellCreator: public IUnknown
{
public:
	STDMETHOD(CreateShell)(IImmersiveShellController** controller) PURE;
};

// Ittr: Needs to exist for legacy or 8.1 codepath
class CImmersiveBehaviorWrapper : public IImmersiveBehavior
{
public:
	CImmersiveBehaviorWrapper(IImmersiveBehavior* behavior);
	~CImmersiveBehaviorWrapper();

	//IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	//IImmersiveBehavior
	STDMETHODIMP OnImmersiveThreadStart(void);
	STDMETHODIMP OnImmersiveThreadStop(void);
	STDMETHODIMP GetMaximumComponentCount(unsigned int* count);
	STDMETHODIMP CreateComponent(unsigned int number, IUnknown** component);
	STDMETHODIMP ShouldCreateComponent(unsigned int number, int* allowed);
private:
	IImmersiveBehavior* m_behavior;
	long m_cRef;
};

void CreateTwinUI();
void CreateTwinUI_UWP();
DWORD WINAPI TwinThread( LPVOID lpParameter );

interface IImmersiveShellHookService : IUnknown
{
	STDMETHOD(Register)(void** a1,
		IImmersiveShellHookService* thiss,
		const unsigned int* prgMessages,
		unsigned int cMessages,
		IUnknown* pNotification, //IImmersiveShellHookNotification
		unsigned int* pdwCookie);//todo:args
	STDMETHOD(Unregister)(UINT cookie);
	STDMETHOD(PostShellHookMessage)(WPARAM wParam, LPARAM lParam);
	STDMETHOD(SetTargetWindowForSerialization)(HWND hwnd);
	STDMETHOD(PostShellHookMessageWithSerialization)(bool a1,
		int a2,
		IImmersiveShellHookService* thiss,
		unsigned int msg,
		int msgParam); //todo:args
	STDMETHOD(UpdateWindowApplicationId)(HWND hwnd, LPCWSTR pszAppID);
	STDMETHOD(HandleWindowReplacement)(HWND hwndOld, HWND hwndNew);
	STDMETHOD_(BOOL, IsExecutionOnSerializedThread)();
};

interface IImmersiveWindowMessageService : IUnknown
{
	STDMETHOD(Register)(UINT msg, void* pNotification, UINT* pdwCookie);
	STDMETHOD(Unregister)(UINT dwCookie);
	STDMETHOD(SendMessageW)(UINT nsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(PostMessageW)(UINT nsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(RequestHotkeys)(); //todo: args
	STDMETHOD(UnrequestHotkeys)(UINT dwCookie);
	STDMETHOD(RequestWTSSessionNotification)(void* pNotification, unsigned int* pdwCookie);
	STDMETHOD(UnrequestWTSSessionNotification)(UINT dwCookie);
	STDMETHOD(RequestPowerSettingNotification)(const GUID* pPowerSettingGuid, void* pNotification, UINT* pdwCookie);
	STDMETHOD(UnrequestPowerSettingNotification)(UINT pdwCookie);
	STDMETHOD(RequestPointerDeviceNotification)(void* pNotification, int notificationType, UINT* pdwCookie);
	STDMETHOD(UnrequestPointerDeviceNotification)(UINT dwCookie);
	STDMETHOD(RegisterDwmIconicThumbnailWindow)();
};

void SetProgmanAsShell();