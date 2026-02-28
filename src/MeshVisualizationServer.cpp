#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "MeshVisualizationServer.h"
#include "RenderProcessor.h"
#include <base/pfGroupData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkProperty.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <chrono>

// 全局渲染会话状态（用于Socket命令操作）
namespace {
    struct RenderSession {
        vtkRenderWindow* renderWindow = nullptr;
        vtkRenderer* renderer = nullptr;
        vtkActorCollection* actors = nullptr;
        vtkActorCollection* edgeActors = nullptr;  // 边界线框叠加层（替代 EdgeVisibility 避免崩溃）
        vtkRenderWindowInteractor* interactor = nullptr;
        void* windowHandle = nullptr;
        const PREPRO_BASE_NAMESPACE::PFData* renderData = nullptr;
        int visualizationType = 1; // 默认网格类型
        bool edgesVisible = true;  // 边界显示状态
        
        void clear() {
            renderWindow = nullptr;
            renderer = nullptr;
            actors = nullptr;
            edgeActors = nullptr;
            interactor = nullptr;
            windowHandle = nullptr;
            renderData = nullptr;
        }
    };
    
    RenderSession g_renderSession;
    std::mutex g_sessionMutex;
    
    // 全局服务器实例指针（用于在SetRenderSession中启动服务器）
    MeshVisualizationServer* g_serverInstance = nullptr;
    std::mutex g_serverInstanceMutex;
}

// 导出函数：获取服务器实例（供 RenderProcessor 访问）
extern "C" {
    MeshVisualizationServer* GetServerInstance() {
        std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
        return g_serverInstance;
    }
    
    void SetServerInstance(MeshVisualizationServer* server) {
        std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
        g_serverInstance = server;
    }
    
    bool IsServerRunning(MeshVisualizationServer* server) {
        if (!server) return false;
        return server->isRunning();
    }
    
    void StopServer(MeshVisualizationServer* server) {
        if (server) {
            server->stop();
        }
    }
}

MeshVisualizationServer::MeshVisualizationServer(int port)
    : m_port(port)
    , m_running(false)
    , m_renderWindowHandle(nullptr)
    , m_renderData(nullptr)
    , m_visualizationType(1)
#ifdef _WIN32
    , m_listenSocket(INVALID_SOCKET)
#else
    , m_listenSocket(-1)
#endif
{
    // 注册为全局服务器实例（用于在SetRenderSession中启动）
    std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
    g_serverInstance = this;
}

MeshVisualizationServer::~MeshVisualizationServer()
{
    stop();
    // 清除全局服务器实例指针
    std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
    if (g_serverInstance == this)
    {
        g_serverInstance = nullptr;
    }
}

bool MeshVisualizationServer::start()
{
    if (m_running)
    {
        return true;
    }

#ifdef _WIN32
    // 初始化Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (result != 0)
    {
        std::cerr << "[MeshVisualizationServer] WSAStartup失败: " << result << std::endl;
        return false;
    }
#endif

    // 创建socket
#ifdef _WIN32
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        std::cerr << "[MeshVisualizationServer] 创建socket失败" << std::endl;
        WSACleanup();
        return false;
    }
#else
    m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenSocket < 0)
    {
        std::cerr << "[MeshVisualizationServer] 创建socket失败" << std::endl;
        return false;
    }
#endif

    // 设置socket选项（允许地址重用）
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
#else
    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
#endif
    {
        std::cerr << "[MeshVisualizationServer] 设置socket选项失败" << std::endl;
#ifdef _WIN32
        closesocket(m_listenSocket);
        WSACleanup();
#else
        close(m_listenSocket);
#endif
        return false;
    }

    // 绑定地址
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(m_port);

#ifdef _WIN32
    if (bind(m_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "[MeshVisualizationServer] 绑定地址失败，端口: " << m_port << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }

    // 开始监听
    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[MeshVisualizationServer] 监听失败" << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }
#else
    if (bind(m_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "[MeshVisualizationServer] 绑定地址失败，端口: " << m_port << std::endl;
        close(m_listenSocket);
        return false;
    }

    if (listen(m_listenSocket, 5) < 0)
    {
        std::cerr << "[MeshVisualizationServer] 监听失败" << std::endl;
        close(m_listenSocket);
        return false;
    }
#endif

    m_running = true;
    m_serverThread = std::thread(&MeshVisualizationServer::serverLoop, this);
    
    std::cout << "[MeshVisualizationServer] 服务器已启动，监听端口: " << m_port << std::endl;
    return true;
}

void MeshVisualizationServer::stop()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;

#ifdef _WIN32
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
#else
    if (m_listenSocket >= 0)
    {
        close(m_listenSocket);
        m_listenSocket = -1;
    }
#endif

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    std::cout << "[MeshVisualizationServer] 服务器已停止" << std::endl;
}

void MeshVisualizationServer::setRenderWindowHandle(void* windowHandle)
{
    // 先更新成员变量（需要m_mutex）
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_renderWindowHandle = windowHandle;
    }
    
    // 然后更新全局会话状态（需要g_sessionMutex）
    // 注意：先释放m_mutex再获取g_sessionMutex，避免死锁
    {
        std::lock_guard<std::mutex> sessionLock(g_sessionMutex);
        g_renderSession.windowHandle = windowHandle;
    }
}

// setRenderData方法已移除，因为PFData不能复制，数据通过SetRenderSession在RenderProcessor中设置

void MeshVisualizationServer::setVisualizationType(int type)
{
    // 先更新成员变量（需要m_mutex）
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_visualizationType = type;
    }
    
    // 然后更新全局会话状态（需要g_sessionMutex）
    // 注意：先释放m_mutex再获取g_sessionMutex，避免死锁
    {
        std::lock_guard<std::mutex> sessionLock(g_sessionMutex);
        g_renderSession.visualizationType = type;
    }
    
    std::cout << "[MeshVisualizationServer] 可视化类型已设置为: " << type << std::endl;
}

void MeshVisualizationServer::serverLoop()
{
    while (m_running)
    {
#ifdef _WIN32
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket == INVALID_SOCKET)
        {
            if (m_running)
            {
                std::cerr << "[MeshVisualizationServer] 接受连接失败" << std::endl;
            }
            continue;
        }
#else
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket < 0)
        {
            if (m_running)
            {
                std::cerr << "[MeshVisualizationServer] 接受连接失败" << std::endl;
            }
            continue;
        }
#endif

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "[MeshVisualizationServer] 客户端连接: " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // 在新线程中处理客户端
        std::thread clientThread(&MeshVisualizationServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void MeshVisualizationServer::handleClient(SOCKET clientSocket)
{
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    
    while (m_running)
    {
        // 接收数据长度（4字节）
        int length = 0;
        int received = receiveData(clientSocket, (char*)&length, sizeof(length));
        if (received != sizeof(length) || length <= 0 || length > BUFFER_SIZE)
        {
            break;
        }
        
        // 接收JSON数据
        received = receiveData(clientSocket, buffer, length);
        if (received != length)
        {
            break;
        }
        
        buffer[length] = '\0';
        
        try
        {
            // 解析JSON命令
            json commandJson = json::parse(buffer);
            
            // 处理命令
            json response = processCommand(commandJson);
            
            // 发送响应
            if (!sendResponse(clientSocket, response))
            {
                break;
            }
        }
        catch (const json::exception& e)
        {
            json errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = std::string("JSON解析错误: ") + e.what();
            sendResponse(clientSocket, errorResponse);
        }
        catch (const std::exception& e)
        {
            json errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = std::string("处理命令时发生错误: ") + e.what();
            sendResponse(clientSocket, errorResponse);
        }
    }
    
#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
    
    std::cout << "[MeshVisualizationServer] 客户端断开连接" << std::endl;
}

json MeshVisualizationServer::processCommand(const json& commandJson)
{
    json response;
    
    if (!commandJson.contains("command") || !commandJson["command"].is_string())
    {
        response["success"] = false;
        response["error"] = "缺少command字段或格式错误";
        return response;
    }
    
    std::string command = commandJson["command"].get<std::string>();
    json params = commandJson.value("params", json::object());
    
    // 注意：不在processCommand中加锁，让每个处理函数自己管理锁，避免死锁
    // 各个处理函数会根据需要获取 g_sessionMutex
    
    if (command == "GetWindowHandle")
    {
        return handleGetWindowHandle(params);
    }
    else if (command == "SetTransparency")
    {
        return handleSetTransparency(params);
    }
    else if (command == "ToggleEdges")
    {
        return handleToggleEdges(params);
    }
    else if (command == "SetRepresentation")
    {
        return handleSetRepresentation(params);
    }
    else if (command == "ToggleWireframe")
    {
        return handleToggleWireframe(params);
    }
    else if (command == "TogglePoints")
    {
        return handleTogglePoints(params);
    }
    else if (command == "ToggleColorByGroup")
    {
        return handleToggleColorByGroup(params);
    }
    else if (command == "ResetCamera")
    {
        return handleResetCamera(params);
    }
    else if (command == "Render")
    {
        return handleRender(params);
    }
    else if (command == "SetSize")
    {
        return handleSetSize(params);
    }
    else
    {
        response["success"] = false;
        response["error"] = "未知命令: " + command;
        return response;
    }
}

json MeshVisualizationServer::handleGetWindowHandle(const json& params)
{
    json response;
    
    // 从全局会话中获取窗口句柄和窗口对象（需要加锁）
    void* windowHandle = nullptr;
    vtkRenderWindow* renderWindow = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        windowHandle = g_renderSession.windowHandle;
        renderWindow = g_renderSession.renderWindow;
    }
    
    // 如果窗口句柄无效，尝试重新获取
    if (!windowHandle && renderWindow)
    {
#ifdef _WIN32
        // 强制渲染以确保窗口创建
        renderWindow->Render();
        
        // 等待窗口创建（最多等待1秒）
        for (int i = 0; i < 20; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            renderWindow->Render();
            windowHandle = reinterpret_cast<void*>(renderWindow->GetGenericWindowId());
            if (windowHandle)
            {
                HWND hwnd = reinterpret_cast<HWND>(windowHandle);
                if (IsWindow(hwnd))
                {
                    // 更新全局会话中的窗口句柄
                    {
                        std::lock_guard<std::mutex> lock(g_sessionMutex);
                        g_renderSession.windowHandle = windowHandle;
                    }
                    break;
                }
            }
        }
#endif
    }
    
    if (windowHandle)
    {
#ifdef _WIN32
        // 验证窗口句柄是否有效
        HWND hwnd = reinterpret_cast<HWND>(windowHandle);
        if (IsWindow(hwnd))
        {
            response["success"] = true;
            response["windowHandle"] = reinterpret_cast<uintptr_t>(windowHandle);
            
            // 获取窗口标题（用于调试）
            char windowTitle[256] = {0};
            GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
            if (strlen(windowTitle) > 0)
            {
                response["windowTitle"] = std::string(windowTitle);
            }
        }
        else
        {
            response["success"] = false;
            response["error"] = "窗口句柄无效（窗口不存在）";
            response["windowHandle"] = reinterpret_cast<uintptr_t>(windowHandle);
        }
#else
        response["success"] = true;
        response["windowHandle"] = reinterpret_cast<uintptr_t>(windowHandle);
#endif
    }
    else
    {
        response["success"] = false;
        response["error"] = "渲染窗口未初始化";
    }
    
    return response;
}

json MeshVisualizationServer::handleSetTransparency(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.actors)
    {
        response["success"] = false;
        response["error"] = "渲染actors未初始化";
        return response;
    }
    
    double opacity = params.value("opacity", 1.0);
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;
    
    g_renderSession.actors->InitTraversal();
    vtkActor* actor = nullptr;
    while ((actor = g_renderSession.actors->GetNextActor()) != nullptr)
    {
        actor->GetProperty()->SetOpacity(opacity);
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["opacity"] = opacity;
    return response;
}

json MeshVisualizationServer::handleToggleEdges(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    // 使用线框叠加层替代 EdgeVisibility，避免 VTK 崩溃
    if (!g_renderSession.edgeActors)
    {
        response["success"] = false;
        response["error"] = "边界叠加层未初始化（需通过主渲染流程创建）";
        return response;
    }
    
    bool enable = params.value("enable", true);
    g_renderSession.edgesVisible = enable;
    
    g_renderSession.edgeActors->InitTraversal();
    vtkActor* actor = nullptr;
    while ((actor = g_renderSession.edgeActors->GetNextActor()) != nullptr)
    {
        if (actor)
            actor->SetVisibility(enable ? 1 : 0);
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["edgesEnabled"] = enable;
    return response;
}

json MeshVisualizationServer::handleSetRepresentation(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.actors)
    {
        response["success"] = false;
        response["error"] = "渲染actors未初始化";
        return response;
    }
    
    std::string representation = params.value("representation", "surface");
    
    g_renderSession.actors->InitTraversal();
    vtkActor* actor = nullptr;
    while ((actor = g_renderSession.actors->GetNextActor()) != nullptr)
    {
        if (representation == "surface")
            actor->GetProperty()->SetRepresentationToSurface();
        else if (representation == "wireframe")
            actor->GetProperty()->SetRepresentationToWireframe();
        else if (representation == "points")
            actor->GetProperty()->SetRepresentationToPoints();
    }
    
    // 线框/点模式下隐藏边界叠加层
    if (g_renderSession.edgeActors)
    {
        bool showEdges = (representation == "surface" && g_renderSession.edgesVisible);
        g_renderSession.edgeActors->InitTraversal();
        vtkActor* edgeActor = nullptr;
        while ((edgeActor = g_renderSession.edgeActors->GetNextActor()) != nullptr)
        {
            if (edgeActor)
                edgeActor->SetVisibility(showEdges ? 1 : 0);
        }
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["representation"] = representation;
    return response;
}

json MeshVisualizationServer::handleToggleWireframe(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.actors)
    {
        response["success"] = false;
        response["error"] = "渲染actors未初始化";
        return response;
    }
    
    bool enable = params.value("enable", true);
    
    g_renderSession.actors->InitTraversal();
    vtkActor* actor = nullptr;
    while ((actor = g_renderSession.actors->GetNextActor()) != nullptr)
    {
        if (enable)
            actor->GetProperty()->SetRepresentationToWireframe();
        else
            actor->GetProperty()->SetRepresentationToSurface();
    }
    
    if (g_renderSession.edgeActors)
    {
        bool showEdges = (!enable && g_renderSession.edgesVisible);
        g_renderSession.edgeActors->InitTraversal();
        vtkActor* edgeActor = nullptr;
        while ((edgeActor = g_renderSession.edgeActors->GetNextActor()) != nullptr)
        {
            if (edgeActor)
                edgeActor->SetVisibility(showEdges ? 1 : 0);
        }
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["wireframeEnabled"] = enable;
    return response;
}

json MeshVisualizationServer::handleTogglePoints(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.actors)
    {
        response["success"] = false;
        response["error"] = "渲染actors未初始化";
        return response;
    }
    
    bool enable = params.value("enable", true);
    
    g_renderSession.actors->InitTraversal();
    vtkActor* actor = nullptr;
    while ((actor = g_renderSession.actors->GetNextActor()) != nullptr)
    {
        if (enable)
        {
            actor->GetProperty()->SetRepresentationToPoints();
            actor->GetProperty()->SetPointSize(5);
        }
        else
        {
            actor->GetProperty()->SetRepresentationToSurface();
        }
    }
    
    if (g_renderSession.edgeActors)
    {
        bool showEdges = (!enable && g_renderSession.edgesVisible);
        g_renderSession.edgeActors->InitTraversal();
        vtkActor* edgeActor = nullptr;
        while ((edgeActor = g_renderSession.edgeActors->GetNextActor()) != nullptr)
        {
            if (edgeActor)
                edgeActor->SetVisibility(showEdges ? 1 : 0);
        }
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["pointsEnabled"] = enable;
    return response;
}

json MeshVisualizationServer::handleToggleColorByGroup(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.actors)
    {
        response["success"] = false;
        response["error"] = "渲染actors未初始化";
        return response;
    }
    
    bool enable = params.value("enable", true);
    
    // 定义一组颜色用于按组着色
    double colors[][3] = {
        {1.0, 0.0, 0.0}, // 红
        {0.0, 1.0, 0.0}, // 绿
        {0.0, 0.0, 1.0}, // 蓝
        {1.0, 1.0, 0.0}, // 黄
        {1.0, 0.0, 1.0}, // 洋红
        {0.0, 1.0, 1.0}, // 青
        {1.0, 0.5, 0.0}, // 橙
        {0.5, 0.0, 1.0}, // 紫
    };
    int numColors = 8;
    
    g_renderSession.actors->InitTraversal();
    vtkActor* actor = nullptr;
    int colorIndex = 0;
    while ((actor = g_renderSession.actors->GetNextActor()) != nullptr)
    {
        if (enable)
        {
            actor->GetProperty()->SetColor(colors[colorIndex % numColors]);
            colorIndex++;
        }
        else
        {
            actor->GetProperty()->SetColor(0.7, 0.7, 0.7); // 恢复默认灰色
        }
    }
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    response["colorByGroupEnabled"] = enable;
    return response;
}

json MeshVisualizationServer::handleResetCamera(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.renderer)
    {
        response["success"] = false;
        response["error"] = "渲染器未初始化";
        return response;
    }
    
    g_renderSession.renderer->ResetCamera();
    
    if (g_renderSession.renderWindow)
    {
        g_renderSession.renderWindow->Render();
    }
    
    response["success"] = true;
    return response;
}

json MeshVisualizationServer::handleRender(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.renderWindow)
    {
        response["success"] = false;
        response["error"] = "渲染窗口未初始化";
        return response;
    }
    
    g_renderSession.renderWindow->Render();
    
    response["success"] = true;
    return response;
}

json MeshVisualizationServer::handleSetSize(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    
    if (!g_renderSession.renderWindow)
    {
        response["success"] = false;
        response["error"] = "渲染窗口未初始化";
        return response;
    }
    
    int width = params.value("width", 800);
    int height = params.value("height", 600);
    
    if (width > 0 && height > 0)
    {
        g_renderSession.renderWindow->SetSize(width, height);
        g_renderSession.renderWindow->Render();
        
        response["success"] = true;
        response["width"] = width;
        response["height"] = height;
    }
    else
    {
        response["success"] = false;
        response["error"] = "无效的窗口尺寸";
    }
    
    return response;
}

bool MeshVisualizationServer::sendResponse(SOCKET clientSocket, const json& response)
{
    std::string responseStr = response.dump();
    int length = static_cast<int>(responseStr.length());
    
    // 发送长度
#ifdef _WIN32
    int sent = send(clientSocket, (char*)&length, sizeof(length), 0);
    if (sent != sizeof(length))
    {
        return false;
    }
    
    // 发送数据
    sent = send(clientSocket, responseStr.c_str(), length, 0);
    return sent == length;
#else
    int sent = send(clientSocket, &length, sizeof(length), 0);
    if (sent != sizeof(length))
    {
        return false;
    }
    
    sent = send(clientSocket, responseStr.c_str(), length, 0);
    return sent == length;
#endif
}

int MeshVisualizationServer::receiveData(SOCKET clientSocket, char* buffer, int bufferSize)
{
    int totalReceived = 0;
    
    while (totalReceived < bufferSize)
    {
#ifdef _WIN32
        int received = recv(clientSocket, buffer + totalReceived, bufferSize - totalReceived, 0);
        if (received <= 0)
        {
            return -1;
        }
#else
        int received = recv(clientSocket, buffer + totalReceived, bufferSize - totalReceived, 0);
        if (received <= 0)
        {
            return -1;
        }
#endif
        totalReceived += received;
    }
    
    return totalReceived;
}

// 导出函数：设置渲染会话（供RenderProcessor调用）
extern "C" {
    void SetRenderSession(vtkRenderWindow* window, vtkRenderer* renderer, 
                         vtkActorCollection* actors, vtkRenderWindowInteractor* interactor,
                         vtkActorCollection* edgeActors = nullptr)
    {
        void* windowHandle = nullptr;
        int visualizationType = 0;
        bool shouldStartServer = false;
        
        // 第一步：在锁内完成所有需要 g_sessionMutex 的操作
        {
            std::lock_guard<std::mutex> lock(g_sessionMutex);
            g_renderSession.renderWindow = window;
            g_renderSession.renderer = renderer;
            g_renderSession.actors = actors;
            g_renderSession.edgeActors = edgeActors;
            g_renderSession.interactor = interactor;
            
            if (window)
            {
#ifdef _WIN32
                // 强制渲染以确保窗口创建
                window->Render();
                
                // 获取窗口句柄，并验证窗口是否有效
                windowHandle = reinterpret_cast<void*>(window->GetGenericWindowId());
                if (windowHandle)
                {
                    HWND hwnd = reinterpret_cast<HWND>(windowHandle);
                    // 如果窗口句柄无效，等待窗口完全创建
                    if (!IsWindow(hwnd))
                    {
                        // 等待窗口创建（最多等待2秒，每50ms检查一次）
                        for (int i = 0; i < 40; ++i)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));
                            // 再次强制渲染
                            window->Render();
                            windowHandle = reinterpret_cast<void*>(window->GetGenericWindowId());
                            hwnd = reinterpret_cast<HWND>(windowHandle);
                            if (IsWindow(hwnd))
                            {
                                std::cout << "[SetRenderSession] 窗口已创建，等待了 " << (i + 1) * 50 << "ms" << std::endl;
                                break;
                            }
                        }
                        if (!IsWindow(hwnd))
                        {
                            std::cerr << "[SetRenderSession] 警告: 窗口句柄无效，窗口可能尚未完全创建" << std::endl;
                            windowHandle = nullptr;
                        }
                    }
                    else
                    {
                        std::cout << "[SetRenderSession] 窗口句柄有效: " << hwnd << std::endl;
                    }
                }
                else
                {
                    std::cerr << "[SetRenderSession] 警告: 无法获取窗口句柄" << std::endl;
                }
#else
                windowHandle = window->GetGenericWindowId();
#endif
                g_renderSession.windowHandle = windowHandle;
            }
            
            visualizationType = g_renderSession.visualizationType;
            
            // 调试信息：输出当前状态
            std::cout << "[SetRenderSession] 调试信息:" << std::endl;
            std::cout << "  windowHandle: " << (windowHandle ? "有效" : "无效") << std::endl;
            std::cout << "  visualizationType: " << visualizationType << std::endl;
            std::cout << "  g_serverInstance: " << (g_serverInstance ? "存在" : "不存在") << std::endl;
            if (g_serverInstance)
            {
                std::cout << "  isRunning: " << (g_serverInstance->isRunning() ? "是" : "否") << std::endl;
            }
            
            // 保存需要的信息，然后释放锁
            shouldStartServer = (windowHandle && visualizationType == 1);
        } // 释放 g_sessionMutex 锁，避免死锁
        
        // 如果窗口已显示且服务器正在运行，设置窗口句柄
        // 注意：服务器已在程序启动时启动，这里只需要设置窗口句柄
        if (windowHandle && visualizationType == 1) // 1 = EMesh
        {
            std::lock_guard<std::mutex> serverLock(g_serverInstanceMutex);
            if (g_serverInstance && g_serverInstance->isRunning())
            {
                try
                {
                    // 设置窗口句柄（此时 g_sessionMutex 已释放，不会死锁）
                    g_serverInstance->setRenderWindowHandle(windowHandle);
                    std::cout << "[SetRenderSession] 窗口句柄已设置到服务器" << std::endl;
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[SetRenderSession] 设置窗口句柄时发生异常: " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "[SetRenderSession] 设置窗口句柄时发生未知异常" << std::endl;
                }
            }
            else
            {
                if (!g_serverInstance)
                {
                    std::cerr << "[SetRenderSession] 警告: 全局服务器实例不存在" << std::endl;
                }
                else if (!g_serverInstance->isRunning())
                {
                    std::cerr << "[SetRenderSession] 警告: 服务器未运行" << std::endl;
                }
            }
        }
    }
}
