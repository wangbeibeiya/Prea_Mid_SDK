#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "SocketCommandAPI.h"
#include "json.hpp"
using json = nlohmann::json;

/**
 * @brief 体可视化服务类
 *
 * 通过 Socket 接收命令，控制体渲染（Volume Rendering）的可视化操作。
 * 与 MeshVisualizationServer（网格表面渲染）互补，用于体数据的三维渲染。
 *
 * 体渲染适用于：后处理标量场、医学影像、CFD 体数据等。
 * 可扩展命令：传递函数、透明度、采样距离、裁剪等。
 */
class VolumeVisualizationServer
{
public:
    /**
     * @brief 构造函数（共享模式：使用外部 SocketCommandAPI，与 MeshVisualizationServer 共用同一端口）
     * @param sharedAPI 共享的 Socket 命令 API，由外部创建并管理生命周期
     */
    explicit VolumeVisualizationServer(SocketCommandAPI* sharedAPI);
    ~VolumeVisualizationServer();

    bool start() const;   ///< 委托给共享 API（由外部调用，此处仅为接口一致）
    void stop() const;
    bool isRunning() const;
    int getPort() const;

    void setRenderWindowHandle(void* windowHandle);
    void* getRenderWindowHandle() const { return m_renderWindowHandle; }

    SocketCommandAPI* getSocketAPI() { return m_sharedAPI; }
    const SocketCommandAPI* getSocketAPI() const { return m_sharedAPI; }

private:
    SocketCommandAPI* m_sharedAPI = nullptr;  ///< 共享的 Socket API（不拥有）
    std::mutex m_mutex;
    void* m_renderWindowHandle = nullptr;

    json handleGetWindowHandle(const json& params);
    json handleResetCamera(const json& params);
    json handleRender(const json& params);
    json handleSetSize(const json& params);
    json handleRenderVolumesWithOpacity(const json& params);
};

// 前向声明 VTK 类型，供外部调用 SetVolumeRenderSession
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;

/**
 * @brief 设置体渲染会话，供体渲染窗口创建后调用
 *
 * 体可视化服务通过此函数获取 renderWindow/renderer/interactor，
 * 以响应 GetWindowHandle、ResetCamera、Render、SetSize 等命令。
 */
void SetVolumeRenderSession(vtkRenderWindow* window, vtkRenderer* renderer,
                            vtkRenderWindowInteractor* interactor);

// 前向声明 vtkVolume
class vtkVolume;

/**
 * @brief 注册体名称与 vtkVolume 的映射，供 RenderVolumesWithOpacity 命令使用
 *
 * 创建 vtkVolume 并添加到 renderer 后，应调用此函数注册体名称，
 * 以便按名称设置透明度（指定体不透明，其他体 50% 透明）。
 */
void RegisterVolumeWithName(vtkVolume* volume, const std::string& volumeName);

// 前向声明 vtkActor
class vtkActor;

/**
 * @brief 设置体名称与 mesh actor 的映射（网格表面渲染模式）
 *
 * 当使用网格渲染（vtkActor）而非体渲染（vtkVolume）时，RenderVolumesWithOpacity
 * 通过此映射按体名称设置 actor 透明度。由 RenderProcessor 在创建渲染窗口时调用。
 */
void SetVolumeActorMapping(const std::unordered_map<std::string, std::vector<vtkActor*>>& volumeActorMap);
