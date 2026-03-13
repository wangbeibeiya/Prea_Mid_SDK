#pragma once

#include <string>
#include <memory>
#include <mutex>

#include "SocketCommandAPI.h"
#include "json.hpp"
using json = nlohmann::json;

// 前向声明
namespace PREPRO_BASE_NAMESPACE {
    class PFData;
}

class RenderProcessor;

/**
 * @brief 网格可视化服务器类
 * 
 * 通过Socket接收命令，控制网格可视化的各种操作
 */
class MeshVisualizationServer
{
public:
    /**
     * @brief 构造函数（独占模式：创建并拥有自己的 Socket）
     * @param port 监听端口，默认54321
     */
    explicit MeshVisualizationServer(int port = 54321);

    /**
     * @brief 构造函数（共享模式：使用外部 SocketCommandAPI，与 VolumeVisualizationServer 共用同一端口）
     * @param sharedAPI 共享的 Socket 命令 API，由外部创建并管理生命周期
     */
    explicit MeshVisualizationServer(SocketCommandAPI* sharedAPI);
    
    /**
     * @brief 析构函数
     */
    ~MeshVisualizationServer();
    
    /**
     * @brief 启动服务器
     * @return 是否成功启动
     */
    bool start();
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 检查服务器是否正在运行
     * @return 是否正在运行
     */
    bool isRunning() const;
    
    /**
     * @brief 设置当前渲染窗口句柄（用于后续操作）
     * @param windowHandle 窗口句柄
     */
    void setRenderWindowHandle(void* windowHandle);
    
    /**
     * @brief 设置渲染类型
     * @param type 可视化类型（0=几何，1=网格，2=后处理）
     */
    void setVisualizationType(int type);
    
    /**
     * @brief 获取当前窗口句柄
     * @return 窗口句柄
     */
    void* getRenderWindowHandle() const { return m_renderWindowHandle; }
    
    /**
     * @brief 获取监听端口
     * @return 端口号
     */
    int getPort() const;

    /**
     * @brief 获取 Socket API 实例，用于扩展注册新命令
     * @return Socket API 指针
     */
    SocketCommandAPI* getSocketAPI() { return getAPI(); }
    const SocketCommandAPI* getSocketAPI() const { return getAPI(); }

private:
    std::unique_ptr<SocketCommandAPI> m_socketAPI;  ///< 独占模式下的 Socket API
    SocketCommandAPI* m_sharedAPI = nullptr;       ///< 共享模式下的 Socket API 指针（不拥有）
    bool m_ownsSocket = true;                       ///< 是否拥有 Socket（共享模式下为 false）
    std::mutex m_mutex;                                   ///< 成员变量互斥锁
    void* m_renderWindowHandle;                           ///< 当前渲染窗口句柄
    const PREPRO_BASE_NAMESPACE::PFData* m_renderData;   ///< 当前渲染数据
    int m_visualizationType;                             ///< 可视化类型
    
    // 命令处理函数
    json handleGetWindowHandle(const json& params);
    json handleSetTransparency(const json& params);
    json handleToggleEdges(const json& params);
    json handleSetRepresentation(const json& params);
    json handleToggleWireframe(const json& params);
    json handleTogglePoints(const json& params);
    json handleToggleColorByGroup(const json& params);
    json handleResetCamera(const json& params);
    json handleRender(const json& params);
    json handleSetSize(const json& params);

    SocketCommandAPI* getAPI() const;  ///< 获取当前使用的 API（独占或共享）
};
