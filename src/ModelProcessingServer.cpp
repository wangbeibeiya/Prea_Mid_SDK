#include "ModelProcessingServer.h"
#include "VolumeProcessor.h"
#include "GeometryAPI.h"
#include "MeshProcessor.h"
#include "Logger.h"
#include "RenderProcessor.h"
#include "MeshVisualizationServer.h"
#include "commonEnumeration.h"
#include "../include/ProjectModelData.h"
#include "../include/Logger.h"
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <mesh/pfMesh.h>
#include <mesh/pfMeshQuality.h>
#include <mesh/pfQualityData.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <thread>
#include <json.hpp>

extern "C" {
    MeshVisualizationServer* GetServerInstance();
}
using json = nlohmann::json;

ModelProcessingServer::ModelProcessingServer(SocketCommandAPI* sharedAPI, Logger* logger)
    : m_sharedAPI(sharedAPI)
    , m_logger(logger)
{
    if (!m_sharedAPI) return;

    m_sharedAPI->registerCommand("ImportGeometryModel", [this](const json& p) { return handleImportGeometryModel(p); });
    m_sharedAPI->registerCommand("ExecuteGeometryProcessing", [this](const json& p) { return handleExecuteGeometryProcessing(p); });
    m_sharedAPI->registerCommand("ExecuteGeometryMatching", [this](const json& p) { return handleExecuteGeometryMatching(p); });
    m_sharedAPI->registerCommand("ExecuteMeshGeneration", [this](const json& p) { return handleExecuteMeshGeneration(p); });
    m_sharedAPI->registerCommand("ShowGeometry", [this](const json& p) { return handleShowGeometry(p); });
    m_sharedAPI->registerCommand("ShowMesh", [this](const json& p) { return handleShowMesh(p); });
    m_sharedAPI->registerCommand("CloseSession", [this](const json& p) { return handleCloseSession(p); });
    m_sharedAPI->registerCommand("DeleteVolumeByName", [this](const json& p) { return handleDeleteVolumeByName(p); });
    m_sharedAPI->registerCommand("GetUnmatchedVolumeNames", [this](const json& p) { return handleGetUnmatchedVolumeNames(p); });
    m_sharedAPI->registerCommand("SavePpcf", [this](const json& p) { return handleSavePpcf(p); });
    m_sharedAPI->registerCommand("GetMeshQuality", [this](const json& p) { return handleGetMeshQuality(p); });
    m_sharedAPI->registerCommand("ImportPpcf", [this](const json& p) { return handleImportPpcf(p); });
}

VolumeProcessor* ModelProcessingServer::getSessionForDataQuery(const std::string& sessionId)
{
    return getSession(sessionId);
}

GeometryAPI* ModelProcessingServer::tryGetGeometryAPIForPath(const std::string& ppcfPath)
{
    if (ppcfPath.empty()) return nullptr;
    std::lock_guard<std::mutex> lock(m_meshSessionMutex);
    std::filesystem::path req(ppcfPath);
    if (m_meshGeometryAPI && !m_meshOpenDocumentPath.empty() && std::filesystem::path(m_meshOpenDocumentPath) == req)
        return m_meshGeometryAPI.get();
    if (!m_meshSessionId.empty() && !m_meshSessionProjectPath.empty() && std::filesystem::path(m_meshSessionProjectPath) == req)
    {
        VolumeProcessor* p = getSession(m_meshSessionId);
        return p ? p->getGeometryAPI() : nullptr;
    }
    return nullptr;
}

GeometryAPI* ModelProcessingServer::tryGetGeometryAPIForMesh()
{
    std::lock_guard<std::mutex> lock(m_meshSessionMutex);
    if (m_meshGeometryAPI && !m_meshOpenDocumentPath.empty())
        return m_meshGeometryAPI.get();
    if (!m_meshSessionId.empty() && !m_meshSessionProjectPath.empty())
    {
        VolumeProcessor* p = getSession(m_meshSessionId);
        return p ? p->getGeometryAPI() : nullptr;
    }
    return nullptr;
}

ModelProcessingServer::~ModelProcessingServer()
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions.clear();
}

std::string ModelProcessingServer::createSession(std::unique_ptr<VolumeProcessor> processor)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    std::string id = "geom_" + std::to_string(++m_nextSessionId);
    m_sessions[id] = std::move(processor);
    return id;
}

VolumeProcessor* ModelProcessingServer::getSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    auto it = m_sessions.find(sessionId);
    return (it != m_sessions.end()) ? it->second.get() : nullptr;
}

void ModelProcessingServer::removeSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions.erase(sessionId);
}

json ModelProcessingServer::handleImportGeometryModel(const json& params)
{
    json response;
    std::string jsonPath = params.value("jsonPath", "");
    if (jsonPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 jsonPath 参数";
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (!std::filesystem::exists(jsonPath))
    {
        response["success"] = false;
        response["error"] = "JSON 文件不存在: " + jsonPath;
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] 失败: " + response["error"].get<std::string>());
        return response;
    }

    ProjectModelData model;
    if (!model.loadFromJsonFile(jsonPath))
    {
        response["success"] = false;
        response["error"] = "加载 JSON 失败: " + jsonPath;
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] JSON 加载失败: " + jsonPath);
        return response;
    }

    std::string workingDir = model.getWorkingDirectory();
    std::string projectName = model.getProjectName();
    if (workingDir.empty() || projectName.empty())
    {
        response["success"] = false;
        response["error"] = "JSON 中缺少 WorkingDirectory 或 ProjectName";
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] 失败: " + response["error"].get<std::string>());
        return response;
    }

    std::filesystem::path modelPath(workingDir);
    modelPath /= projectName;
    modelPath /= projectName + ".stp";
    if (!std::filesystem::exists(modelPath))
    {
        response["success"] = false;
        response["error"] = "模型文件不存在: " + modelPath.string();
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] 模型不存在: " + modelPath.string());
        return response;
    }

    auto processor = std::make_unique<VolumeProcessor>("", m_logger);
    if (!processor->initialize(false))
    {
        response["success"] = false;
        response["error"] = "SDK 初始化失败";
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] SDK 初始化失败");
        return response;
    }

    if (!processor->importGeometryModel(modelPath.string(), 1, &model))
    {
        response["success"] = false;
        response["error"] = processor->getLastError();
        if (m_logger) m_logger->logOutputLine("[ImportGeometryModel] 导入失败: " + processor->getLastError());
        return response;
    }

    std::string sessionId = createSession(std::move(processor));
    response["success"] = true;
    response["message"] = "几何模型导入成功";
    response["sessionId"] = sessionId;
    return response;
}

json ModelProcessingServer::handleExecuteGeometryProcessing(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数（请先调用 ImportGeometryModel）";
        return response;
    }

    VolumeProcessor* processor = getSession(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryProcessing] 失败: " + response["error"].get<std::string>());
        return response;
    }

    VolumeProcessor::ProcessOptions options;
    options.enableQuickRepair = params.value("enableQuickRepair", true);
    options.enableFindVolumes = params.value("enableFindVolumes", true);

    if (processor->executeGeometryProcessing(options))
    {
        response["success"] = true;
        response["message"] = "几何处理完成";

    }
    else
    {
        response["success"] = false;
        response["error"] = processor->getLastError();
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryProcessing] 失败: " + processor->getLastError());
    }
    return response;
}

json ModelProcessingServer::handleExecuteGeometryMatching(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    std::string jsonPath = params.value("jsonPath", "");
    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数（请先调用 ImportGeometryModel）";
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (jsonPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 jsonPath 参数";
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (!std::filesystem::exists(jsonPath))
    {
        response["success"] = false;
        response["error"] = "JSON 文件不存在: " + jsonPath;
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 失败: " + response["error"].get<std::string>());
        return response;
    }

    VolumeProcessor* processor = getSession(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 失败: " + response["error"].get<std::string>());
        return response;
    }

    ProjectModelData model;
    if (!model.loadFromJsonFile(jsonPath))
    {
        response["success"] = false;
        response["error"] = "加载 JSON 失败: " + jsonPath;
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] JSON 加载失败: " + jsonPath);
        return response;
    }

    if (!processor->executeGeometryMatching(&model))
    {
        response["success"] = false;
        response["error"] = processor->getLastError();
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 失败: " + processor->getLastError());
        return response;
    }

    std::string workingDir = model.getWorkingDirectory();
    std::string projectName = model.getProjectName();
    if (!workingDir.empty() && !projectName.empty())
    {
        std::filesystem::path cacheDir(workingDir);
        cacheDir /= projectName;
        if (model.saveVolumeRenameMapToCache(cacheDir.string()))
        {
            if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] VolumeRenameMap 已保存到缓存");
        }
    }

    if (!processor->refreshGeometryData())
    {
        response["success"] = false;
        response["error"] = "刷新顶层数据失败: " + processor->getLastError();
        if (m_logger) m_logger->logOutputLine("[ExecuteGeometryMatching] 刷新失败: " + processor->getLastError());
        return response;
    }

    // 保留 session，供 GetVolumeListNames、SavePpcf 等从顶层数据查询
    response["success"] = true;
    response["message"] = "几何识别匹配完成，session 保留可继续查询。请调用 SavePpcf 保存 ppcf 文件。";
    response["sessionId"] = sessionId;
    return response;
}

json ModelProcessingServer::handleShowGeometry(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数（请先调用 ImportGeometryModel）";
        return response;
    }

    VolumeProcessor* processor = getSession(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        return response;
    }

    GeometryAPI* api = processor->getGeometryAPI();
    if (!api)
    {
        response["success"] = false;
        response["error"] = "无法获取几何 API";
        return response;
    }

    PREPRO_BASE_NAMESPACE::PFData data;
    if (!api->getAllData(data))
    {
        response["success"] = false;
        response["error"] = "获取几何数据失败: " + api->getLastError();
        return response;
    }

    if (data.getGroupSize() == 0 && data.getVolumeSize() == 0)
    {
        response["success"] = false;
        response["error"] = "几何数据为空，无法显示";
        return response;
    }

    MeshVisualizationServer* meshServer = GetServerInstance();
    if (meshServer)
    {
        meshServer->setVisualizationType(static_cast<int>(VisualizationType::EGeomerty));
    }

    std::string sid = sessionId;
    std::thread([this, sid]() {
        try
        {
            VolumeProcessor* proc = getSession(sid);
            if (!proc)
            {
                std::cerr << "[ShowGeometry] 会话已过期: " << sid << std::endl;
                return;
            }
            GeometryAPI* geometryApi = proc->getGeometryAPI();
            if (!geometryApi) return;
            PREPRO_BASE_NAMESPACE::PFData threadData;
            if (!geometryApi->getAllData(threadData)) return;
            RenderProcessor::show(threadData, VisualizationType::EGeomerty);
        }
        catch (const std::exception& e)
        {
            std::cerr << "[ShowGeometry] 渲染异常: " << e.what() << std::endl;
        }
    }).detach();

    response["success"] = true;
    response["message"] = "几何可视化窗口正在启动，请稍候。窗口出现后可使用 RenderVolumesWithOpacity 等命令。";
    response["sessionId"] = sessionId;
    return response;
}

json ModelProcessingServer::handleShowMesh(const json& params)
{
    json response;
    std::string ppcfPath = params.value("ppcfPath", "");

    if (!ppcfPath.empty())
    {
        if (!std::filesystem::exists(ppcfPath))
        {
            response["success"] = false;
            response["error"] = "ppcf 文件不存在: " + ppcfPath;
            if (m_logger) m_logger->logOutputLine("[ShowMesh] 失败: " + response["error"].get<std::string>());
            return response;
        }
    }
    else
    {
        if (!tryGetGeometryAPIForMesh())
        {
            response["success"] = false;
            response["error"] = "未传入 ppcfPath 且无缓存的网格会话（请先执行 ExecuteMeshGeneration）";
            if (m_logger) m_logger->logOutputLine("[ShowMesh] 失败: " + response["error"].get<std::string>());
            return response;
        }
    }

    std::thread([this, ppcfPath]() {
        try
        {
            GeometryAPI* apiToUse = nullptr;
            std::unique_ptr<GeometryAPI> geometryAPIOwned;

            if (!ppcfPath.empty())
            {
                // 传入了 ppcfPath：直接使用该文件（或复用已打开的同一路径）
                if (!std::filesystem::exists(ppcfPath))
                {
                    if (m_logger) m_logger->logOutputLine("[ShowMesh] 失败: ppcf 文件不存在: " + ppcfPath);
                    return;
                }
                apiToUse = tryGetGeometryAPIForPath(ppcfPath);
                if (!apiToUse)
                {
                    geometryAPIOwned = std::make_unique<GeometryAPI>();
                    if (!geometryAPIOwned->initialize())
                    {
                        if (m_logger) m_logger->logOutputLine("[ShowMesh] GeometryAPI 初始化失败");
                        return;
                    }
                    if (!geometryAPIOwned->openDocument(ppcfPath))
                    {
                        if (m_logger) m_logger->logOutputLine("[ShowMesh] 打开 ppcf 失败: " + geometryAPIOwned->getLastError());
                        return;
                    }
                    apiToUse = geometryAPIOwned.get();
                }
                else if (m_logger)
                    m_logger->logOutputLine("[ShowMesh] 复用 GeometryAPI（ppcf 已打开）");
            }
            else
            {
                // 未传入 ppcfPath：复用 ExecuteMeshGeneration 后的 GeometryAPI
                apiToUse = tryGetGeometryAPIForMesh();
                if (!apiToUse)
                {
                    if (m_logger) m_logger->logOutputLine("[ShowMesh] 失败: 未传入 ppcfPath 且无缓存的网格会话（请先执行 ExecuteMeshGeneration）");
                    return;
                }
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 复用 GeometryAPI（网格会话）");
            }
            auto* pfDocument = apiToUse->getDocument();
            if (!pfDocument)
            {
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 无法获取文档");
                return;
            }
            MeshProcessor meshProcessor(pfDocument);
            if (!meshProcessor.initialize())
            {
                if (m_logger) m_logger->logOutputLine("[ShowMesh] MeshProcessor 初始化失败");
                return;
            }
            PREPRO_BASE_NAMESPACE::PFData data;
            if (!meshProcessor.getMeshData(data))
            {
                auto* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
                if (!pfGeometry || pfGeometry->getAllData(data) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                {
                    if (m_logger) m_logger->logOutputLine("[ShowMesh] 无法获取网格/几何数据");
                    return;
                }
            }
            if (data.getGroupSize() == 0 && data.getVolumeSize() == 0)
            {
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 数据为空，无法显示");
                return;
            }
            MeshVisualizationServer* meshServer = GetServerInstance();
            if (meshServer)
            {
                meshServer->setVisualizationType(static_cast<int>(VisualizationType::EMesh));
            }
            RenderProcessor::show(data, VisualizationType::EMesh);
        }
        catch (const std::exception& e)
        {
            if (m_logger) m_logger->logOutputLine("[ShowMesh] 异常: " + std::string(e.what()));
        }
    }).detach();

    response["success"] = true;
    response["message"] = "网格可视化窗口正在启动，请稍候。窗口出现后可使用 RenderMesh、ToggleMeshEdges 等命令。";
    response["ppcfPath"] = ppcfPath;
    if (m_logger) m_logger->logOutputLine("[ShowMesh] 已启动网格可视化");
    return response;
}

json ModelProcessingServer::handleCloseSession(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数";
        return response;
    }

    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        if (m_meshSessionId == sessionId)
        {
            m_meshSessionId.clear();
            m_meshSessionProjectPath.clear();
        }
    }
    removeSession(sessionId);
    response["success"] = true;
    response["message"] = "会话已关闭";
    return response;
}

json ModelProcessingServer::handleDeleteVolumeByName(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    std::string volumeName = params.value("volumeName", "");

    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数";
        if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (volumeName.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 volumeName 参数";
        if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 失败: " + response["error"].get<std::string>());
        return response;
    }

    VolumeProcessor* processor = getSession(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 失败: " + response["error"].get<std::string>());
        return response;
    }

    GeometryAPI* api = processor->getGeometryAPI();
    if (!api)
    {
        response["success"] = false;
        response["error"] = "无法获取几何 API";
        if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 失败: " + response["error"].get<std::string>());
        return response;
    }

    if (!api->deleteVolumes({ volumeName }))
    {
        response["success"] = false;
        response["error"] = "删除体失败: " + api->getLastError();
        if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 失败: " + response["error"].get<std::string>());
        return response;
    }

    response["success"] = true;
    response["message"] = "已删除体: " + volumeName;
    response["sessionId"] = sessionId;
    response["deletedVolume"] = volumeName;
    if (m_logger) m_logger->logOutputLine("[DeleteVolumeByName] 成功删除体: " + volumeName);
    return response;
}

json ModelProcessingServer::handleGetUnmatchedVolumeNames(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");

    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数（请先调用 ImportGeometryModel 并执行 ExecuteGeometryMatching）";
        return response;
    }

    VolumeProcessor* processor = getSession(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        return response;
    }

    const auto& unmatched = processor->getLastUnmatchedVolumeNames();
    response["success"] = true;
    response["sessionId"] = sessionId;
    response["unmatchedVolumeNames"] = unmatched;
    response["count"] = static_cast<int>(unmatched.size());
    return response;
}

json ModelProcessingServer::handleExecuteMeshGeneration(const json& params)
{
    json response;
    std::string jsonPath = params.value("jsonPath", "");
    std::string ppcfPath = params.value("ppcfPath", "");
    std::string sessionId = params.value("sessionId", "");
    if (jsonPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 jsonPath 参数";
        return response;
    }
    if (!std::filesystem::exists(jsonPath))
    {
        response["success"] = false;
        response["error"] = "JSON 文件不存在: " + jsonPath;
        return response;
    }
    // 有 sessionId 时复用几何 session 的 GeometryAPI，可不保存 ppcf；无 sessionId 时需 ppcf 存在
    if (sessionId.empty() && !ppcfPath.empty() && !std::filesystem::exists(ppcfPath))
    {
        response["success"] = false;
        response["error"] = "ppcf 文件不存在: " + ppcfPath;
        return response;
    }

    GeometryAPI* geometryAPIIn = nullptr;
    std::string projectPpcfPath;
    if (!sessionId.empty())
    {
        VolumeProcessor* processor = getSession(sessionId);
        if (!processor)
        {
            response["success"] = false;
            response["error"] = "会话不存在或已过期: " + sessionId;
            return response;
        }
        geometryAPIIn = processor->getGeometryAPI();
        if (!geometryAPIIn)
        {
            response["success"] = false;
            response["error"] = "会话中无 GeometryAPI";
            return response;
        }
        ProjectModelData model;
        if (!model.loadFromJsonFile(jsonPath))
        {
            response["success"] = false;
            response["error"] = "加载 JSON 失败: " + jsonPath;
            return response;
        }
        std::string wd = model.getWorkingDirectory(), pn = model.getProjectName();
        if (!wd.empty() && !pn.empty())
        {
            std::filesystem::path p(wd);
            p /= pn;
            p /= pn + ".ppcf";
            projectPpcfPath = p.string();
        }
        if (projectPpcfPath.empty())
            projectPpcfPath = ppcfPath;
    }

    std::string err;
    std::string ppcfPathToUse = sessionId.empty() ? ppcfPath : projectPpcfPath;
    bool ok = geometryAPIIn
        ? ModelProcessingServer::executeMeshGenerationFromJson(jsonPath, &err, ppcfPathToUse, m_logger, geometryAPIIn, nullptr, nullptr)
        : ModelProcessingServer::executeMeshGenerationFromJson(jsonPath, &err, ppcfPathToUse, m_logger, nullptr, &m_meshGeometryAPI, &m_meshOpenDocumentPath);
    if (ok)
    {
        response["success"] = true;
        response["message"] = "网格划分完成";
        if (geometryAPIIn)
        {
            std::lock_guard<std::mutex> lock(m_meshSessionMutex);
            m_meshSessionId = sessionId;
            m_meshSessionProjectPath = projectPpcfPath;
            m_meshGeometryAPI.reset();  // 使用 session 的，不保留独立副本
            m_meshOpenDocumentPath.clear();
        }
    }
    else
    {
        response["success"] = false;
        response["error"] = err;
        if (m_logger) m_logger->logOutputLine("[ExecuteMeshGeneration] 失败: " + err);
    }
    return response;
}

bool ModelProcessingServer::executeGeometryMatchingFromJson(const std::string& jsonPath, std::string* errorOut)
{
    VolumeProcessor processor;
    if (!processor.initialize(false))
    {
        if (errorOut) *errorOut = "SDK 初始化失败";
        return false;
    }

    if (!processor.processGeometryMatchingFromJson(jsonPath))
    {
        if (errorOut) *errorOut = processor.getLastError();
        return false;
    }
    return true;
}

namespace {

bool saveMeshPpcfFromDocument(GeometryAPI& geometryAPI, const std::string& savePath, std::string* errorOut)
{
    std::filesystem::path p(savePath);
    std::filesystem::create_directories(p.parent_path());
    if (!geometryAPI.saveDocument(savePath))
    {
        if (errorOut) *errorOut = "保存 ppcf 失败: " + geometryAPI.getLastError();
        return false;
    }
    return true;
}

} // namespace

bool ModelProcessingServer::saveMeshPpcfToPath(const std::string& sourcePpcfPath, const std::string& savePathParam, std::string* errorOut)
{
    if (sourcePpcfPath.empty())
    {
        if (errorOut) *errorOut = "源 ppcf 路径为空";
        return false;
    }
    if (!std::filesystem::exists(sourcePpcfPath))
    {
        if (errorOut) *errorOut = "源 ppcf 文件不存在: " + sourcePpcfPath;
        return false;
    }

    std::string savePath = savePathParam.empty() ? sourcePpcfPath : savePathParam;

    GeometryAPI geometryAPI;
    if (!geometryAPI.initialize())
    {
        if (errorOut) *errorOut = "GeometryAPI 初始化失败: " + geometryAPI.getLastError();
        return false;
    }
    if (!geometryAPI.openDocument(sourcePpcfPath))
    {
        if (errorOut) *errorOut = "加载 ppcf 失败: " + geometryAPI.getLastError();
        return false;
    }
    return saveMeshPpcfFromDocument(geometryAPI, savePath, errorOut);
}

json ModelProcessingServer::handleSavePpcf(const json& params)
{
    json response;
    std::string savePath = params.value("savePath", "");
    if (savePath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 savePath 参数（保存路径）";
        if (m_logger) m_logger->logOutputLine("[SavePpcf] 失败: " + response["error"].get<std::string>());
        return response;
    }

    std::string err;
    GeometryAPI* apiToSave = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        if (m_meshGeometryAPI && m_meshGeometryAPI->getDocument())
        {
            apiToSave = m_meshGeometryAPI.get();
        }
        else if (!m_meshSessionId.empty())
        {
            VolumeProcessor* p = getSession(m_meshSessionId);
            if (p && p->getGeometryAPI() && p->getGeometryAPI()->getDocument())
                apiToSave = p->getGeometryAPI();
        }
    }

    if (!apiToSave)
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        for (const auto& kv : m_sessions)
        {
            if (kv.second)
            {
                GeometryAPI* api = kv.second->getGeometryAPI();
                if (api && api->getDocument())
                {
                    apiToSave = api;
                    break;
                }
            }
        }
    }

    if (apiToSave)
    {
        std::filesystem::path p(savePath);
        std::filesystem::create_directories(p.parent_path());
        if (apiToSave->saveDocument(savePath))
        {
            response["success"] = true;
            response["message"] = "ppcf 已保存（有啥保存啥：有几何保存几何，有网格+几何保存网格+几何）";
            response["savePath"] = savePath;
            if (m_logger) m_logger->logOutputLine("[SavePpcf] 成功: " + savePath);
        }
        else
        {
            response["success"] = false;
            response["error"] = "保存失败: " + apiToSave->getLastError();
            if (m_logger) m_logger->logOutputLine("[SavePpcf] 失败: " + response["error"].get<std::string>());
        }
    }
    else
    {
        response["success"] = false;
        response["error"] = "无可用文档可保存（请先执行 ImportGeometryModel、ExecuteGeometryMatching、ExecuteMeshGeneration 或 ImportPpcf）";
        if (m_logger) m_logger->logOutputLine("[SavePpcf] 失败: " + response["error"].get<std::string>());
    }
    return response;
}

json ModelProcessingServer::handleGetMeshQuality(const json& params)
{
    json response;
    std::string ppcfPath = params.value("ppcfPath", "");
    if (ppcfPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 ppcfPath 参数（包含网格的 ppcf 文件路径）";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (!std::filesystem::exists(ppcfPath))
    {
        response["success"] = false;
        response["error"] = "ppcf 文件不存在: " + ppcfPath;
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: " + response["error"].get<std::string>());
        return response;
    }

    GeometryAPI* apiToUse = tryGetGeometryAPIForPath(ppcfPath);
    std::unique_ptr<GeometryAPI> geometryAPIOwned;
    if (!apiToUse)
    {
        geometryAPIOwned = std::make_unique<GeometryAPI>();
        if (!geometryAPIOwned->initialize())
        {
            response["success"] = false;
            response["error"] = "GeometryAPI 初始化失败";
            if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: GeometryAPI 初始化失败");
            return response;
        }
        if (!geometryAPIOwned->openDocument(ppcfPath))
        {
            response["success"] = false;
            response["error"] = "打开 ppcf 失败: " + geometryAPIOwned->getLastError();
            if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: " + response["error"].get<std::string>());
            return response;
        }
        apiToUse = geometryAPIOwned.get();
    }

    auto* pfDocument = apiToUse->getDocument();
    if (!pfDocument)
    {
        response["success"] = false;
        response["error"] = "无法获取文档";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: 无法获取文档");
        return response;
    }

    auto* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (!pfMesh)
    {
        response["success"] = false;
        response["error"] = "无法获取网格环境";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: 无法获取网格环境");
        return response;
    }

    auto* pfMeshQuality = pfMesh->getMeshQuality();
    if (!pfMeshQuality)
    {
        response["success"] = false;
        response["error"] = "无法获取网格质量数据";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: 无法获取网格质量数据");
        return response;
    }

    auto* pfMetricQuality = pfMeshQuality->getQualityData();
    if (!pfMetricQuality || pfMetricQuality->getQualityCount() == 0)
    {
        response["success"] = false;
        response["error"] = "无网格质量数据";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: 无网格质量数据");
        return response;
    }

    const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfMetricQuality->getQualityData(0);
    response["success"] = true;
    response["ppcfPath"] = ppcfPath;
    response["elementCount"] = quality->getElementCount();
    response["invalidCount"] = quality->getInvalidCount();
    const float* qv = quality->getQualityValue();
    response["worstQuality"] = qv[0];
    response["bestQuality"] = qv[1];
    response["averageQuality"] = qv[2];
    json countsArr = json::array();
    int qualityCount = quality->getQualityCount();
    const int* counts = quality->getCounts();
    for (int i = qualityCount - 1; i >= 0; --i)
    {
        json rangeObj;
        rangeObj["range"] = std::to_string(i * 0.1) + " < Q <= " + std::to_string((i + 1) * 0.1);
        rangeObj["count"] = counts[i];
        countsArr.push_back(rangeObj);
    }
    response["qualityRanges"] = countsArr;
    if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 成功: " + ppcfPath);
    return response;
}

json ModelProcessingServer::handleImportPpcf(const json& params)
{
    json response;
    std::string ppcfPath = params.value("ppcfPath", "");
    std::string importMode = params.value("importMode", "geometry");
    if (ppcfPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 ppcfPath 参数";
        if (m_logger) m_logger->logOutputLine("[ImportPpcf] 失败: " + response["error"].get<std::string>());
        return response;
    }
    if (!std::filesystem::exists(ppcfPath))
    {
        response["success"] = false;
        response["error"] = "ppcf 文件不存在: " + ppcfPath;
        if (m_logger) m_logger->logOutputLine("[ImportPpcf] 失败: " + response["error"].get<std::string>());
        return response;
    }
    bool useOpenDocument = (importMode != "mesh");

    auto geometryAPI = std::make_unique<GeometryAPI>();
    if (!geometryAPI->initialize())
    {
        response["success"] = false;
        response["error"] = "GeometryAPI 初始化失败";
        if (m_logger) m_logger->logOutputLine("[ImportPpcf] 失败: GeometryAPI 初始化失败");
        return response;
    }

    bool opened = useOpenDocument ? geometryAPI->openDocument(ppcfPath) : geometryAPI->importMesh(ppcfPath);
    if (!opened)
    {
        response["success"] = false;
        response["error"] = std::string(useOpenDocument ? "openDocument" : "importMesh") + " 失败: " + geometryAPI->getLastError();
        if (m_logger) m_logger->logOutputLine("[ImportPpcf] 失败: " + response["error"].get<std::string>());
        return response;
    }

    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        m_meshGeometryAPI = std::move(geometryAPI);
        m_meshOpenDocumentPath = ppcfPath;
        m_meshSessionId.clear();
        m_meshSessionProjectPath.clear();
    }

    response["success"] = true;
    response["message"] = "ppcf 已导入，顶层数据已刷新。后续 GetVolumeListNames、GetFaceGroupNamesByVolume、ShowMesh、GetMeshQuality 等可使用此 ppcfPath 或复用当前会话。";
    response["ppcfPath"] = ppcfPath;
    response["importMode"] = useOpenDocument ? "geometry" : "mesh";
    if (m_logger) m_logger->logOutputLine("[ImportPpcf] 成功: " + ppcfPath);
    return response;
}

bool ModelProcessingServer::executeMeshGenerationFromJson(const std::string& jsonPath, std::string* errorOut, const std::string& ppcfPathParam, Logger* logger,
    GeometryAPI* geometryAPIIn, std::unique_ptr<GeometryAPI>* meshSessionOut, std::string* meshSessionPathOut)
{
    ProjectModelData model;
    if (!model.loadFromJsonFile(jsonPath))
    {
        if (errorOut) *errorOut = "加载 JSON 失败: " + jsonPath;
        return false;
    }

    std::string workingDir = model.getWorkingDirectory();
    std::string projectName = model.getProjectName();

    std::string ppcfPathStr;
    if (!ppcfPathParam.empty())
    {
        ppcfPathStr = ppcfPathParam;
    }
    else
    {
        if (workingDir.empty() || projectName.empty())
        {
            if (errorOut) *errorOut = "JSON 中缺少 WorkingDirectory 或 ProjectName";
            return false;
        }
        std::filesystem::path derived(workingDir);
        derived /= projectName;
        derived /= projectName + ".ppcf";
        ppcfPathStr = derived.string();
    }

    if (!geometryAPIIn && !std::filesystem::exists(ppcfPathStr))
    {
        if (errorOut) *errorOut = "几何 ppcf 文件不存在: " + ppcfPathStr;
        return false;
    }

    // 几何识别需要 SetList，若 model 中为空则从 project.json 补充
    if (model.GetSetList().empty() && !workingDir.empty() && !projectName.empty())
    {
        std::filesystem::path projectJsonPath(workingDir);
        projectJsonPath /= projectName;
        projectJsonPath /= projectName + ".json";
        if (std::filesystem::exists(projectJsonPath))
        {
            try
            {
                std::ifstream f(projectJsonPath);
                if (f.is_open())
                {
                    json j;
                    f >> j;
                    f.close();
                    if (j.contains("SetList") && j["SetList"].is_array())
                    {
                        std::vector<std::shared_ptr<SetItemBase>> setList;
                        for (const auto& setJson : j["SetList"])
                        {
                            std::string setType = setJson.value("SetType", "");
                            std::shared_ptr<SetItemBase> set;
                            if (setType == "Solid")
                            {
                                auto solidSet = std::make_shared<SetSolidItem>();
                                solidSet->fromJson(setJson);
                                set = solidSet;
                            }
                            else if (setType == "Face")
                            {
                                auto faceSet = std::make_shared<SetFaceItem>();
                                faceSet->fromJson(setJson);
                                set = faceSet;
                            }
                            else
                            {
                                auto solidSet = std::make_shared<SetSolidItem>();
                                solidSet->fromJson(setJson);
                                set = solidSet;
                            }
                            if (set) setList.push_back(set);
                        }
                        model.SetSetList(setList);
                    }
                }
            }
            catch (...) {}
        }
    }

    if (!workingDir.empty() && !projectName.empty())
    {
        std::filesystem::path cacheDir(workingDir);
        cacheDir /= projectName;
        model.loadVolumeRenameMapFromCache(cacheDir.string());
    }

    try
    {
        std::unique_ptr<GeometryAPI> geometryAPI;
        GeometryAPI* apiToUse = nullptr;
        if (geometryAPIIn)
        {
            apiToUse = geometryAPIIn;
        }
        else
        {
            geometryAPI = std::make_unique<GeometryAPI>();
            if (!geometryAPI->initialize())
            {
                if (errorOut) *errorOut = "GeometryAPI 初始化失败: " + geometryAPI->getLastError();
                return false;
            }
            if (!geometryAPI->openDocument(ppcfPathStr))
            {
                if (errorOut) *errorOut = "加载几何 ppcf 失败: " + geometryAPI->getLastError();
                return false;
            }
            apiToUse = geometryAPI.get();
        }

        auto* pfDocument = apiToUse->getDocument();
        if (!pfDocument)
        {
            if (errorOut) *errorOut = "无法获取文档";
            return false;
        }

        MeshProcessor meshProcessor(pfDocument);
        if (!meshProcessor.initialize())
        {
            if (errorOut) *errorOut = "网格处理器初始化失败: " + meshProcessor.getLastError();
            return false;
        }

        if (logger)
        {
            meshProcessor.setProgressCallback([logger](const std::string& msg) {
                logger->logOutputLine("[MeshProcessor] " + msg);
            });
        }

        MeshProcessor::MeshParameters meshParams;
        std::vector<LocalFluidMeshItem> localMeshItems = model.getLocalFluidMeshsInfo();

        // 辅助：从 FluidMeshInfo 填充 meshParams
        auto applyFluidMeshInfo = [&meshParams](const FluidMeshInfo& info) {
            if (info.MinMeshSize.has_value())
                meshParams.minSize = info.MinMeshSize.value() / 1000.0;
            if (info.MaxMeshSize.has_value())
                meshParams.maxSize = info.MaxMeshSize.value() / 1000.0;
            if (info.GrowthRate.has_value())
                meshParams.growthRate = info.GrowthRate.value();
            if (info.NormalAngle.has_value())
                meshParams.curvatureNormalAngle = info.NormalAngle.value();
        };

        // 1. 先从 jsonPath 加载的 model 获取全局网格参数
        applyFluidMeshInfo(model.getFluidMeshInfo());

        // 2. 尝试从项目目录 JSON (WorkingDirectory/ProjectName/ProjectName.json) 补充/覆盖
        try
        {
            std::filesystem::path jsonFilePath(workingDir);
            jsonFilePath /= projectName;
            jsonFilePath /= projectName + ".json";
            if (std::filesystem::exists(jsonFilePath))
            {
                std::ifstream jsonFile(jsonFilePath);
                if (jsonFile.is_open())
                {
                    json originalJson;
                    jsonFile >> originalJson;
                    jsonFile.close();

                    // 2a. 全局网格 FluidMeshInfo：project.json 有则覆盖（项目配置优先）
                    if (originalJson.contains("FluidMeshInfo") && originalJson["FluidMeshInfo"].is_object())
                    {
                        FluidMeshInfo projectFluidInfo;
                        projectFluidInfo.fromJson(originalJson["FluidMeshInfo"]);
                        applyFluidMeshInfo(projectFluidInfo);
                    }

                    // 2b. 局部网格 LocalFluidMeshsInfo：model 为空时从 project.json 加载
                    if (localMeshItems.empty() && originalJson.contains("LocalFluidMeshsInfo") && originalJson["LocalFluidMeshsInfo"].is_object())
                    {
                        const auto& lfmInfo = originalJson["LocalFluidMeshsInfo"];
                        if (lfmInfo.contains("LocalFluidMeshsInfo") && lfmInfo["LocalFluidMeshsInfo"].is_array())
                        {
                            for (const auto& itemJson : lfmInfo["LocalFluidMeshsInfo"])
                            {
                                LocalFluidMeshItem lfmItem;
                                lfmItem.fromJson(itemJson);
                                localMeshItems.push_back(lfmItem);
                            }
                        }
                    }

                    // 2c. SetList：局部网格需要 SetList 匹配 RefinementSet，model 为空时从 project.json 加载
                    if (!localMeshItems.empty() && model.GetSetList().empty() && originalJson.contains("SetList") && originalJson["SetList"].is_array())
                    {
                        std::vector<std::shared_ptr<SetItemBase>> setList;
                        for (const auto& setJson : originalJson["SetList"])
                        {
                            std::string setType = setJson.value("SetType", "");
                            std::shared_ptr<SetItemBase> set;
                            if (setType == "Solid")
                            {
                                auto solidSet = std::make_shared<SetSolidItem>();
                                solidSet->fromJson(setJson);
                                set = solidSet;
                            }
                            else if (setType == "Face")
                            {
                                auto faceSet = std::make_shared<SetFaceItem>();
                                faceSet->fromJson(setJson);
                                set = faceSet;
                            }
                            else
                            {
                                auto solidSet = std::make_shared<SetSolidItem>();
                                solidSet->fromJson(setJson);
                                set = solidSet;
                            }
                            if (set) setList.push_back(set);
                        }
                        model.SetSetList(setList);
                    }

                    // 2d. 边界层参数（支持 FluidMeshInfo.BoundaryLayer、FluidMeshInfo.BoundaryLayersInfo、根级 BoundaryLayersInfo）
                    const json* blObj = nullptr;
                    if (originalJson.contains("FluidMeshInfo") && originalJson["FluidMeshInfo"].is_object())
                    {
                        const auto& fm = originalJson["FluidMeshInfo"];
                        if (fm.contains("BoundaryLayersInfo") && fm["BoundaryLayersInfo"].is_object())
                            blObj = &fm["BoundaryLayersInfo"];
                        else if (fm.contains("BoundaryLayer") && fm["BoundaryLayer"].is_object())
                            blObj = &fm["BoundaryLayer"];
                    }
                    if (!blObj && originalJson.contains("BoundaryLayersInfo") && originalJson["BoundaryLayersInfo"].is_object())
                        blObj = &originalJson["BoundaryLayersInfo"];
                    if (blObj)
                    {
                        const auto& bl = *blObj;
                        MeshProcessor::BoundaryLayerParameters blParams;
                        // FirstLayerHeight 在 UI/JSON 中单位已是 m，无需换算（与 MinMeshSize 不同，后者为 mm）
                        if (bl.contains("FirstLayerHeight")) blParams.firstLayerHeight = bl["FirstLayerHeight"].get<double>();
                        if (bl.contains("GrowthRate")) blParams.growthRate = bl["GrowthRate"].get<double>();
                        if (bl.contains("LayersNumber")) blParams.layersNumber = bl["LayersNumber"].get<int>();
                        if (bl.contains("IsBoundaryLayers")) blParams.isBoundaryLayers = bl["IsBoundaryLayers"].get<bool>();
                        if (bl.contains("FluidZoneSet") && !bl["FluidZoneSet"].is_null())
                        {
                            meshParams.fluidZoneSetName = bl["FluidZoneSet"].get<std::string>();
                            if (blParams.isBoundaryLayers)
                            {
                                meshParams.boundaryLayerParams = blParams;
                                if (originalJson.contains("BoundaryConditionInfo") && originalJson["BoundaryConditionInfo"].is_object())
                                {
                                    const auto& bci = originalJson["BoundaryConditionInfo"];
                                    if (bci.contains("BoundaryConditions") && bci["BoundaryConditions"].is_array())
                                    {
                                        for (const auto& bc : bci["BoundaryConditions"])
                                        {
                                            std::string bcType = bc.value("Type", "");
                                            std::string bcName = bc.value("Name", "");
                                            if (bcType != "Wall" && bcType != "wall" && !bcName.empty())
                                                meshParams.excludedBoundaryNames.insert(bcName);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (const std::exception&) {}

        meshProcessor.setGlobalParameters(meshParams, &model);
        if (!localMeshItems.empty())
        {
            meshProcessor.setLocalMeshParameters(localMeshItems, &model);
        }

        if (!meshProcessor.createVolumeMeshByGeometry(true, true))
        {
            if (errorOut) *errorOut = "网格生成失败: " + meshProcessor.getLastError();
            return false;
        }

        if (!meshProcessor.hasMeshData())
        {
            if (logger) logger->logOutputLine("[ExecuteMeshGeneration] 警告: createVolumeMeshByGeometry 成功但无网格数据");
        }

        if (!saveMeshPpcfFromDocument(*apiToUse, ppcfPathStr, errorOut))
            return false;

        if (!geometryAPIIn && meshSessionOut && meshSessionPathOut)
        {
            *meshSessionOut = std::move(geometryAPI);
            *meshSessionPathOut = ppcfPathStr;
        }
        return true;
    }
    catch (const std::exception& e)
    {
        if (errorOut) *errorOut = std::string("网格划分异常: ") + e.what();
        return false;
    }
}
