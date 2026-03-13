#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class vtkRenderWindowInteractor;

/**
 * @brief OpenGL 渲染必须在创建上下文的线程执行。
 * Socket 命令在独立线程处理，直接调用 Render() 会导致 wglMakeCurrent 失败。
 * 本模块提供将渲染工作投递到 VTK 主线程的机制。
 */
namespace RenderThreadBridge {

/// 将渲染工作投递到 VTK 线程执行并等待完成（阻塞调用线程）
/// work 会在 VTK 交互器线程中执行
void SubmitAndWait(std::function<void()> work);

/// 在 VTK 交互器上注册定时器，用于处理待执行的渲染工作
/// 必须在 SetRenderSession 时调用，且 interactor 尚未 Start
void ScheduleProcessTimer(vtkRenderWindowInteractor* interactor);

} // namespace RenderThreadBridge
