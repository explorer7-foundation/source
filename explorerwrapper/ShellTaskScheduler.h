#pragma once
#define INITGUID
#include "common.h"

#pragma region GUID definitions
DEFINE_GUID(IID_IShellTaskScheduler7, 0x6CCB7BE0, 0x6807, 0x11D0, 0x0B8, 0x10, 0x0, 0x0C0, 0x4F, 0x0D7, 0x6, 0x0EC); //{6CCB7BE0-6807-11D0-B810-00C04FD706EC}
DEFINE_GUID(IID_IShellTaskSchedulerSettings7, 0x4BC6CE0A, 0x2B39, 0x4F63, 0x89, 0x0C1, 0x3B, 0x0EA, 0x0A7, 0x0BD, 0x0E0, 0x2A); //_GUID_4bc6ce0a_2b39_4f63_89c1_3beaa7bde02a
DEFINE_GUID(IID_IShellTaskSchedulerSettings8, 0x0A8272E00, 0x0A569, 0x40D2, 0x9D, 0x0AC, 0x0B7, 0x75, 0x6F, 0x0A0, 0x92, 0xC4); //_GUID_a8272e00_a569_40d2_9dac_b7756fa092c4
#pragma endregion

/*
CShellTaskScheduler::QueryInterface(_GUID const &,void * *)
CShellTaskScheduler::AddRef(void)
CShellTaskScheduler::Release(void)
CShellTaskScheduler::AddTask(IRunnableTask *,_GUID const &,unsigned __int64,ulong)
CShellTaskScheduler::RemoveTasks(_GUID const &,unsigned __int64,int)
CShellTaskScheduler::CountTasks(_GUID const &)
CShellTaskScheduler::Status(ulong,ulong)
CShellTaskScheduler::AddTask2(IRunnableTask *,_GUID const &,unsigned __int64,ulong,ulong)
CShellTaskScheduler::MoveTask(_GUID const &,unsigned __int64,ulong,ulong)
*/
MIDL_INTERFACE("6CCB7BE0-6807-11D0-B810-00C04FD706EC")
IShellTaskScheduler7
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;
    STDMETHOD(AddTask)(IRunnableTask*,_GUID const&,unsigned __int64,ULONG) PURE;
    STDMETHOD(RemoveTasks)(_GUID const&,unsigned __int64,int) PURE;
    STDMETHOD_(__int64, CountTasks)(_GUID const&) PURE;
    STDMETHOD(Status)(ULONG, ULONG) PURE;
    STDMETHOD(AddTask2)(IRunnableTask*,_GUID const&,unsigned __int64, ULONG, ULONG) PURE;
    STDMETHOD(MoveTask)(_GUID const&,unsigned __int64, ULONG, ULONG) PURE;
};

MIDL_INTERFACE("4bc6ce0a-2b39-4f63-89c1-3beaa7bde02a")
IShellTaskSchedulerSettings7
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;
    STDMETHOD(SetWorkerThreadCountMax)(int) PURE;
    STDMETHOD(SetWorkerThreadPriority)(int) PURE;
    STDMETHOD(SetFlags)(DWORD,DWORD) PURE;
    STDMETHOD(GetFlags)(DWORD*) PURE;
};

MIDL_INTERFACE("a8272e00-a569-40d2-9dac-b7756fa092c4")
IShellTaskSchedulerSettings8
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;
    STDMETHOD(SetWorkerThreadCountMax)(int) PURE;
    STDMETHOD(GetWorkerThreadCountMax)(int*) PURE;
    STDMETHOD(SetWorkerThreadPriority)(int) PURE;
    STDMETHOD(SetFlags)(DWORD,DWORD) PURE;
    STDMETHOD(GetFlags)(DWORD*) PURE;
};

class CShellTaskSchedulerSettingsWrapper : public IShellTaskSchedulerSettings7
{
public:

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP SetWorkerThreadCountMax(int);
    STDMETHODIMP SetWorkerThreadPriority(int);
    STDMETHODIMP SetFlags(DWORD, DWORD);
    STDMETHODIMP GetFlags(DWORD*);

    IShellTaskSchedulerSettings8* m_TaskSchedulerSettings;

    CShellTaskSchedulerSettingsWrapper(IShellTaskSchedulerSettings8* actualInstance)
    {
        m_TaskSchedulerSettings = actualInstance;
    }
};

class CShellTaskSchedulerWrapper : public IShellTaskScheduler7
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP AddTask(IRunnableTask*, _GUID const&, unsigned __int64, ULONG);
    STDMETHODIMP RemoveTasks(_GUID const&, unsigned __int64, int);
    STDMETHODIMP_(__int64) CountTasks(_GUID const&);
    STDMETHODIMP Status(ULONG, ULONG);
    STDMETHODIMP AddTask2(IRunnableTask*, _GUID const&, unsigned __int64, ULONG, ULONG);
    STDMETHODIMP MoveTask(_GUID const&, unsigned __int64, ULONG, ULONG);

    IShellTaskScheduler7* m_TaskScheduler;

    CShellTaskSchedulerWrapper(IShellTaskScheduler7* actualInstance)
    {
        m_TaskScheduler = actualInstance;
    };
};