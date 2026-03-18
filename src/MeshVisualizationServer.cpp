#include "MeshVisualizationServer.h"
#include "RenderProcessor.h"
#include "VolumeVisualizationServer.h"
#include "RenderThreadBridge.h"
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

    void ClearRenderSessionOnWindowClose() {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        g_renderSession.renderWindow = nullptr;
        g_renderSession.renderer = nullptr;
        g_renderSession.actors = nullptr;
        g_renderSession.edgeActors = nullptr;
        g_renderSession.interactor = nullptr;
        g_renderSession.windowHandle = nullptr;
        g_renderSession.renderData = nullptr;
        ClearVolumeRenderSession();
    }
}

MeshVisualizationServer::MeshVisualizationServer(int port)
    : m_renderWindowHandle(nullptr)
    , m_renderData(nullptr)
    , m_visualizationType(1)
    , m_ownsSocket(true)
{
    m_socketAPI = std::make_unique<SocketCommandAPI>(port);
    SocketCommandAPI* api = m_socketAPI.get();

    // 注册网格相关命令（Mesh 写在函数名中）
    api->registerCommand("GetMeshWindowHandle", [this](const json& p) { return handleGetWindowHandle(p); });
    api->registerCommand("ResetMeshCamera", [this](const json& p) { return handleResetCamera(p); });
    api->registerCommand("RenderMesh", [this](const json& p) { return handleRender(p); });
    api->registerCommand("SetMeshSize", [this](const json& p) { return handleSetSize(p); });
    api->registerCommand("ToggleMeshEdges", [this](const json& p) { return handleToggleEdges(p); });
    api->registerCommand("SetMeshRepresentation", [this](const json& p) { return handleSetRepresentation(p); });
    api->registerCommand("ToggleMeshWireframe", [this](const json& p) { return handleToggleWireframe(p); });
    api->registerCommand("ToggleMeshPoints", [this](const json& p) { return handleTogglePoints(p); });
    api->registerCommand("SetMeshTransparency", [this](const json& p) { return handleSetTransparency(p); });
    api->registerCommand("ToggleMeshColorByGroup", [this](const json& p) { return handleToggleColorByGroup(p); });

    std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
    g_serverInstance = this;
}

MeshVisualizationServer::MeshVisualizationServer(SocketCommandAPI* sharedAPI)
    : m_renderWindowHandle(nullptr)
    , m_renderData(nullptr)
    , m_visualizationType(1)
    , m_sharedAPI(sharedAPI)
    , m_ownsSocket(false)
{
    if (!m_sharedAPI) return;

    // 注册网格相关命令（Mesh 写在函数名中）
    m_sharedAPI->registerCommand("GetMeshWindowHandle", [this](const json& p) { return handleGetWindowHandle(p); });
    m_sharedAPI->registerCommand("ResetMeshCamera", [this](const json& p) { return handleResetCamera(p); });
    m_sharedAPI->registerCommand("RenderMesh", [this](const json& p) { return handleRender(p); });
    m_sharedAPI->registerCommand("SetMeshSize", [this](const json& p) { return handleSetSize(p); });
    m_sharedAPI->registerCommand("ToggleMeshEdges", [this](const json& p) { return handleToggleEdges(p); });
    m_sharedAPI->registerCommand("SetMeshRepresentation", [this](const json& p) { return handleSetRepresentation(p); });
    m_sharedAPI->registerCommand("ToggleMeshWireframe", [this](const json& p) { return handleToggleWireframe(p); });
    m_sharedAPI->registerCommand("ToggleMeshPoints", [this](const json& p) { return handleTogglePoints(p); });
    m_sharedAPI->registerCommand("SetMeshTransparency", [this](const json& p) { return handleSetTransparency(p); });
    m_sharedAPI->registerCommand("ToggleMeshColorByGroup", [this](const json& p) { return handleToggleColorByGroup(p); });

    std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
    g_serverInstance = this;
}

MeshVisualizationServer::~MeshVisualizationServer()
{
    if (m_ownsSocket) stop();
    // 清除全局服务器实例指针
    std::lock_guard<std::mutex> lock(g_serverInstanceMutex);
    if (g_serverInstance == this)
    {
        g_serverInstance = nullptr;
    }
}

SocketCommandAPI* MeshVisualizationServer::getAPI() const
{
    return m_ownsSocket ? m_socketAPI.get() : m_sharedAPI;
}

bool MeshVisualizationServer::start()
{
    SocketCommandAPI* api = getAPI();
    return api && api->start();
}

void MeshVisualizationServer::stop()
{
    SocketCommandAPI* api = getAPI();
    if (api) api->stop();
}

bool MeshVisualizationServer::isRunning() const
{
    SocketCommandAPI* api = getAPI();
    return api && api->isRunning();
}

int MeshVisualizationServer::getPort() const
{
    SocketCommandAPI* api = getAPI();
    return api ? api->getPort() : 0;
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

json MeshVisualizationServer::handleGetWindowHandle(const json& params)
{
    json response;
    
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    void* windowHandle = g_renderSession.windowHandle;
    
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
            response["error"] = "窗口句柄无效（窗口已关闭）";
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
            SetVolumeRenderSession(window, renderer, interactor);
            g_renderSession.edgeActors = edgeActors;
            g_renderSession.interactor = interactor;
            RenderThreadBridge::ScheduleProcessTimer(interactor);
            
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
