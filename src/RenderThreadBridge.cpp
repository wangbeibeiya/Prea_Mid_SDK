#include "../include/RenderThreadBridge.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <chrono>

namespace RenderThreadBridge {

namespace {
    std::mutex g_workMutex;
    std::function<void()> g_pendingWork;
    std::condition_variable g_workDoneCond;
    std::atomic<bool> g_hasPendingWork{false};
}

void SubmitAndWait(std::function<void()> work)
{
    if (!work) return;

    std::unique_lock<std::mutex> lock(g_workMutex);
    g_pendingWork = std::move(work);
    g_hasPendingWork = true;

    // 等待 VTK 线程处理完成（最多 5 秒）
    g_workDoneCond.wait_for(lock, std::chrono::seconds(5), [] {
        return !g_hasPendingWork;
    });
}

void ProcessPendingWork()
{
    std::function<void()> work;
    {
        std::lock_guard<std::mutex> lock(g_workMutex);
        if (!g_hasPendingWork || !g_pendingWork) return;
        work = std::move(g_pendingWork);
        g_pendingWork = nullptr;
    }

    if (work)
    {
        work();
    }

    {
        std::lock_guard<std::mutex> lock(g_workMutex);
        g_hasPendingWork = false;
    }
    g_workDoneCond.notify_one();
}

static void TimerCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
{
    (void)caller;
    (void)eventId;
    (void)clientData;
    (void)callData;
    ProcessPendingWork();
}

void ScheduleProcessTimer(vtkRenderWindowInteractor* interactor)
{
    if (!interactor) return;
    vtkSmartPointer<vtkCallbackCommand> cb = vtkSmartPointer<vtkCallbackCommand>::New();
    cb->SetCallback(TimerCallback);
    interactor->AddObserver(vtkCommand::TimerEvent, cb);
    interactor->CreateRepeatingTimer(50);
}

} // namespace RenderThreadBridge
