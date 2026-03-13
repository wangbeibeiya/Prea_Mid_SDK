#include "VolumeVisualizationServer.h"
#include "RenderThreadBridge.h"
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeCollection.h>
#include <unordered_map>
#include <string>
#include <iostream>

namespace {
    struct VolumeRenderSession {
        vtkRenderWindow* renderWindow = nullptr;
        vtkRenderer* renderer = nullptr;
        vtkRenderWindowInteractor* interactor = nullptr;
        void* windowHandle = nullptr;
        std::unordered_map<std::string, vtkVolume*> volumeNameMap;  ///< 体名称 -> vtkVolume 映射
        std::unordered_map<std::string, std::vector<vtkActor*>> volumeActorMap;  ///< 体名称 -> mesh actors（网格模式）
    };
    VolumeRenderSession g_volumeSession;
    std::mutex g_volumeSessionMutex;
}

VolumeVisualizationServer::VolumeVisualizationServer(SocketCommandAPI* sharedAPI)
    : m_renderWindowHandle(nullptr)
    , m_sharedAPI(sharedAPI)
{
    if (!m_sharedAPI) return;

    // 注册体渲染相关命令（Volume 写在函数名中）
    m_sharedAPI->registerCommand("GetVolumeWindowHandle", [this](const json& p) { return handleGetWindowHandle(p); });
    m_sharedAPI->registerCommand("ResetVolumeCamera", [this](const json& p) { return handleResetCamera(p); });
    m_sharedAPI->registerCommand("RenderVolume", [this](const json& p) { return handleRender(p); });
    m_sharedAPI->registerCommand("SetVolumeSize", [this](const json& p) { return handleSetSize(p); });
    m_sharedAPI->registerCommand("RenderVolumesWithOpacity", [this](const json& p) { return handleRenderVolumesWithOpacity(p); });
}

VolumeVisualizationServer::~VolumeVisualizationServer()
{
    // 不停止共享 Socket，由外部管理
}

bool VolumeVisualizationServer::start() const
{
    return m_sharedAPI && m_sharedAPI->start();
}

void VolumeVisualizationServer::stop() const
{
    if (m_sharedAPI) m_sharedAPI->stop();
}

bool VolumeVisualizationServer::isRunning() const
{
    return m_sharedAPI && m_sharedAPI->isRunning();
}

int VolumeVisualizationServer::getPort() const
{
    return m_sharedAPI ? m_sharedAPI->getPort() : 0;
}

void VolumeVisualizationServer::setRenderWindowHandle(void* windowHandle)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_renderWindowHandle = windowHandle;
    std::lock_guard<std::mutex> sessionLock(g_volumeSessionMutex);
    g_volumeSession.windowHandle = windowHandle;
}

json VolumeVisualizationServer::handleGetWindowHandle(const json& params)
{
    json response;
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    if (g_volumeSession.windowHandle)
    {
        response["success"] = true;
        response["windowHandle"] = reinterpret_cast<uintptr_t>(g_volumeSession.windowHandle);
    }
    else
    {
        response["success"] = false;
        response["error"] = "体渲染窗口未初始化";
    }
    return response;
}

json VolumeVisualizationServer::handleResetCamera(const json& params)
{
    json response;
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    if (!g_volumeSession.renderer)
    {
        response["success"] = false;
        response["error"] = "体渲染未初始化";
        return response;
    }
    g_volumeSession.renderer->ResetCamera();
    if (g_volumeSession.renderWindow)
    {
        g_volumeSession.renderWindow->Render();
    }
    response["success"] = true;
    return response;
}

json VolumeVisualizationServer::handleRender(const json& params)
{
    json response;
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    if (!g_volumeSession.renderWindow)
    {
        response["success"] = false;
        response["error"] = "体渲染未初始化";
        return response;
    }
    g_volumeSession.renderWindow->Render();
    response["success"] = true;
    return response;
}

json VolumeVisualizationServer::handleSetSize(const json& params)
{
    json response;
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    if (!g_volumeSession.renderWindow)
    {
        response["success"] = false;
        response["error"] = "体渲染窗口未初始化";
        return response;
    }
    int width = params.value("width", 800);
    int height = params.value("height", 600);
    if (width > 0 && height > 0)
    {
        g_volumeSession.renderWindow->SetSize(width, height);
        g_volumeSession.renderWindow->Render();
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

void SetVolumeRenderSession(vtkRenderWindow* window, vtkRenderer* renderer,
                            vtkRenderWindowInteractor* interactor)
{
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    g_volumeSession.renderWindow = window;
    g_volumeSession.renderer = renderer;
    g_volumeSession.interactor = interactor;
    g_volumeSession.volumeActorMap.clear();
    if (window)
    {
        g_volumeSession.windowHandle = reinterpret_cast<void*>(window->GetGenericWindowId());
    }
}

void SetVolumeActorMapping(const std::unordered_map<std::string, std::vector<vtkActor*>>& volumeActorMap)
{
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    g_volumeSession.volumeActorMap = volumeActorMap;
}

void RegisterVolumeWithName(vtkVolume* volume, const std::string& volumeName)
{
    if (!volume || volumeName.empty()) return;
    std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
    g_volumeSession.volumeNameMap[volumeName] = volume;
}

static void setVolumeScalarOpacity(vtkVolume* volume, double opacity)
{
    if (!volume || !volume->GetProperty()) return;
    vtkPiecewiseFunction* ofun = vtkPiecewiseFunction::New();
    ofun->AddPoint(0.0, opacity);
    ofun->AddPoint(1.0, opacity);
    volume->GetProperty()->SetScalarOpacity(0, ofun);
    ofun->Delete();
}

static void setVolumeColor(vtkVolume* volume, double r, double g, double b)
{
    if (!volume || !volume->GetProperty()) return;
    vtkColorTransferFunction* ctf = vtkColorTransferFunction::New();
    ctf->AddRGBPoint(0.0, r, g, b);
    ctf->AddRGBPoint(1.0, r, g, b);
    volume->GetProperty()->SetColor(ctf);
    ctf->Delete();
}

json VolumeVisualizationServer::handleRenderVolumesWithOpacity(const json& params)
{
    json response;
    std::string focusVolumeName = params.value("volumeName", "");
    double otherOpacity = 0.5;
    if (params.contains("opacity"))
    {
        if (params["opacity"].is_number())
            otherOpacity = params["opacity"].get<double>();
        else if (params["opacity"].is_string())
            otherOpacity = std::stod(params["opacity"].get<std::string>());
    }
    if (otherOpacity < 0.0) otherOpacity = 0.0;
    if (otherOpacity > 1.0) otherOpacity = 1.0;

    {
        std::lock_guard<std::mutex> lock(g_volumeSessionMutex);
        if (!g_volumeSession.renderer)
        {
            response["success"] = false;
            response["error"] = "体渲染未初始化";
            return response;
        }
        if (!g_volumeSession.renderWindow)
        {
            response["success"] = false;
            response["error"] = "体渲染窗口未初始化";
            return response;
        }
    }

    const double opaqueOpacity = 1.0;

    // OpenGL 操作必须在 VTK 线程执行，避免 wglMakeCurrent 失败
    RenderThreadBridge::SubmitAndWait([&response, focusVolumeName, opaqueOpacity, otherOpacity]() {
        std::lock_guard<std::mutex> lock(g_volumeSessionMutex);

        if (!g_volumeSession.volumeActorMap.empty())
        {
            int processedCount = 0;
            bool focusFound = false;
            const double focusColor[3] = {1.0, 0.85, 0.2};   // 金黄色高亮
            const double otherColor[3] = {0.7, 0.7, 0.7};   // 灰色
            for (const auto& pair : g_volumeSession.volumeActorMap)
            {
                const std::string& volName = pair.first;
                const std::vector<vtkActor*>& actors = pair.second;
                bool isFocus = (!focusVolumeName.empty() && volName == focusVolumeName);
                if (isFocus) focusFound = true;
                double opacity = isFocus ? opaqueOpacity : otherOpacity;
                const double* color = isFocus ? focusColor : otherColor;
                for (vtkActor* actor : actors)
                {
                    if (actor && actor->GetProperty())
                    {
                        actor->GetProperty()->SetOpacity(opacity);
                        actor->GetProperty()->SetColor(color[0], color[1], color[2]);
                        processedCount++;
                    }
                }
            }
            if (g_volumeSession.renderWindow)
                g_volumeSession.renderWindow->Render();
            response["success"] = true;
            response["processedCount"] = processedCount;
            response["focusVolumeName"] = focusVolumeName;
            response["mode"] = "mesh";
            if (!focusVolumeName.empty() && !focusFound)
                response["warning"] = "未找到指定体名称 \"" + focusVolumeName + "\"，所有体已设为 50% 透明度。";
            return;
        }

        vtkVolumeCollection* volumes = g_volumeSession.renderer->GetVolumes();
        if (!volumes || volumes->GetNumberOfItems() == 0)
        {
            response["success"] = false;
            response["error"] = "渲染器中无体对象。请先执行 ExecuteMeshGeneration 或导入几何/网格以创建渲染窗口。";
            return;
        }

        volumes->InitTraversal();
        vtkVolume* vol = nullptr;
        int processedCount = 0;
        bool focusFound = false;
        while ((vol = volumes->GetNextVolume()) != nullptr)
        {
            if (!vol) continue;
            bool isFocus = false;
            if (!focusVolumeName.empty())
            {
                for (const auto& p : g_volumeSession.volumeNameMap)
                {
                    if (p.second == vol && p.first == focusVolumeName)
                    {
                        isFocus = true;
                        focusFound = true;
                        break;
                    }
                }
            }
            double opacity = isFocus ? opaqueOpacity : otherOpacity;
            setVolumeScalarOpacity(vol, opacity);
            if (isFocus)
                setVolumeColor(vol, 1.0, 0.85, 0.2);  // 金黄色高亮
            else
                setVolumeColor(vol, 0.7, 0.7, 0.7);   // 灰色
            processedCount++;
        }
        if (g_volumeSession.renderWindow)
            g_volumeSession.renderWindow->Render();
        response["success"] = true;
        response["processedCount"] = processedCount;
        response["focusVolumeName"] = focusVolumeName;
        response["mode"] = "volume";
        if (!focusVolumeName.empty() && !focusFound)
            response["warning"] = "未找到指定体名称 \"" + focusVolumeName + "\"，所有体已设为 50% 透明度。请确保已调用 RegisterVolumeWithName 注册体名称。";
    });

    return response;
}
