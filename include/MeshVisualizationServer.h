#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <map>

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
     * @brief 构造函数
     * @param port 监听端口，默认54321
     */
    explicit MeshVisualizationServer(int port = 54321);
    
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
    bool isRunning() const { return m_running; }
    
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
    int getPort() const { return m_port; }

private:
    int m_port;                                    ///< 监听端口
    std::atomic<bool> m_running;                  ///< 服务器运行状态
    std::thread m_serverThread;                   ///< 服务器线程
    std::mutex m_mutex;                            ///< 互斥锁
    
    void* m_renderWindowHandle;                    ///< 当前渲染窗口句柄
    const PREPRO_BASE_NAMESPACE::PFData* m_renderData; ///< 当前渲染数据
    int m_visualizationType;                      ///< 可视化类型
    
    SOCKET m_listenSocket;                         ///< 监听socket
#ifdef _WIN32
    WSADATA m_wsaData;                             ///< Winsock数据
#endif
    
    /**
     * @brief 服务器主循环
     */
    void serverLoop();
    
    /**
     * @brief 处理客户端连接
     * @param clientSocket 客户端socket
     */
    void handleClient(SOCKET clientSocket);
    
    /**
     * @brief 处理命令
     * @param commandJson 命令JSON对象
     * @return 响应JSON对象
     */
    json processCommand(const json& commandJson);
    
    /**
     * @brief 发送响应
     * @param clientSocket 客户端socket
     * @param response 响应JSON对象
     */
    bool sendResponse(SOCKET clientSocket, const json& response);
    
    /**
     * @brief 接收数据
     * @param clientSocket 客户端socket
     * @param buffer 接收缓冲区
     * @param bufferSize 缓冲区大小
     * @return 接收到的字节数，-1表示错误
     */
    int receiveData(SOCKET clientSocket, char* buffer, int bufferSize);
    
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
};
