#include "ShellTaskScheduler.h"
#include "dbgprint.h"

HRESULT __stdcall CShellTaskSchedulerWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
    if (riid == IID_IShellTaskSchedulerSettings7)
    {
        IShellTaskSchedulerSettings8* taskSettings;
        HRESULT res = m_TaskScheduler->QueryInterface(IID_IShellTaskSchedulerSettings8,(void**)&taskSettings);
        if (res == S_OK)
        {
            dbgprintf(L"CShellTaskSchedulerWrapper::QueryInterface S_OK!!\n");
            *ppvObject = new CShellTaskSchedulerSettingsWrapper(taskSettings);
            return res;
        }

    }
    return m_TaskScheduler->QueryInterface(riid,ppvObject);
}

ULONG __stdcall CShellTaskSchedulerWrapper::AddRef(void)
{
    return m_TaskScheduler->AddRef();
}

ULONG __stdcall CShellTaskSchedulerWrapper::Release(void)
{
    return m_TaskScheduler->Release();
}

HRESULT __stdcall CShellTaskSchedulerWrapper::AddTask(IRunnableTask* a1, _GUID const& a2, unsigned __int64 a3, ULONG a4)
{
    return m_TaskScheduler->AddTask(a1,a2,a3,a4);
}

HRESULT __stdcall CShellTaskSchedulerWrapper::RemoveTasks(_GUID const& a1, unsigned __int64 a2, int a3)
{
    return m_TaskScheduler->RemoveTasks(a1,a2,a3);
}

__int64 __stdcall CShellTaskSchedulerWrapper::CountTasks(_GUID const& a1)
{
    return m_TaskScheduler->CountTasks(a1);
}

HRESULT __stdcall CShellTaskSchedulerWrapper::Status(ULONG a1, ULONG a2)
{
    return m_TaskScheduler->Status(a1,a2);
}

HRESULT __stdcall CShellTaskSchedulerWrapper::AddTask2(IRunnableTask* a1, _GUID const& a2, unsigned __int64 a3, ULONG a4, ULONG a5)
{
    return m_TaskScheduler->AddTask2(a1,a2,a3,a4,a5);
}

HRESULT __stdcall CShellTaskSchedulerWrapper::MoveTask(_GUID const& a1, unsigned __int64 a2, ULONG a3, ULONG a4)
{
    return m_TaskScheduler->MoveTask(a1,a2,a3,a4);
}

HRESULT __stdcall CShellTaskSchedulerSettingsWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
    return m_TaskSchedulerSettings->QueryInterface(riid,ppvObject);
}

ULONG __stdcall CShellTaskSchedulerSettingsWrapper::AddRef(void)
{
    return m_TaskSchedulerSettings->AddRef();
}

ULONG __stdcall CShellTaskSchedulerSettingsWrapper::Release(void)
{
    return m_TaskSchedulerSettings->Release();
}

HRESULT __stdcall CShellTaskSchedulerSettingsWrapper::SetWorkerThreadCountMax(int a1)
{
    return m_TaskSchedulerSettings->SetWorkerThreadCountMax(a1);
}

HRESULT __stdcall CShellTaskSchedulerSettingsWrapper::SetWorkerThreadPriority(int a1)
{
    return m_TaskSchedulerSettings->SetWorkerThreadPriority(a1);
}

HRESULT __stdcall CShellTaskSchedulerSettingsWrapper::SetFlags(DWORD a1, DWORD a2)
{
    return m_TaskSchedulerSettings->SetFlags(a1,a2);
}

HRESULT __stdcall CShellTaskSchedulerSettingsWrapper::GetFlags(DWORD* a1)
{
    return m_TaskSchedulerSettings->GetFlags(a1);
}
