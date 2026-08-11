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
#include <chrono>
#include <iomanip>
#include <thread>
#include <set>
#include <map>
#include <json.hpp>

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellType.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkIntArray.h>
#include <vtkStringArray.h>
#include <vtkCellData.h>

extern "C" {
    MeshVisualizationServer* GetServerInstance();
}
using json = nlohmann::json;

namespace {

static bool geometryApiHasReadableMeshData(GeometryAPI* api)
{
    if (!api) return false;
    auto* pfDocument = api->getDocument();
    if (!pfDocument) return false;
    MeshProcessor meshProcessor(pfDocument);
    if (!meshProcessor.initialize()) return false;
    PREPRO_BASE_NAMESPACE::PFData data;
    if (!meshProcessor.getMeshData(data)) return false;
    return data.getGroupSize() > 0 || data.getVolumeSize() > 0;
}

static bool reloadMeshGeometryApiFromPpcf(const std::string& ppcfPath, std::unique_ptr<GeometryAPI>& outApi, std::string* errorOut)
{
    outApi.reset();
    auto api = std::make_unique<GeometryAPI>();
    if (!api->initialize())
    {
        if (errorOut) *errorOut = "GeometryAPI 初始化失败: " + api->getLastError();
        return false;
    }
    // 优先 openDocument（几何+网格一体化 ppcf 的常见路径）；若仍读不到网格，再回退 importMesh（兼容“网格 ppcf/网格侧存储”场景）
    if (!api->openDocument(ppcfPath))
    {
        if (errorOut) *errorOut = "openDocument 失败: " + api->getLastError();
        return false;
    }
    if (!geometryApiHasReadableMeshData(api.get()))
    {
        if (!api->importMesh(ppcfPath))
        {
            if (errorOut) *errorOut = "openDocument 后无网格数据且 importMesh 失败: " + api->getLastError();
            return false;
        }
        if (!geometryApiHasReadableMeshData(api.get()))
        {
            if (errorOut) *errorOut = "importMesh 后仍未读取到网格数据: " + ppcfPath;
            return false;
        }
    }
    outApi = std::move(api);
    return true;
}

} // namespace

static bool exportMeshPfDataToVtu(const PREPRO_BASE_NAMESPACE::PFData& data, const std::string& vtuPath, std::string* errorOut)
{
    try
    {
        std::filesystem::path outPath(vtuPath);
        if (outPath.empty())
        {
            if (errorOut) *errorOut = "vtuPath 为空";
            return false;
        }
        if (outPath.has_parent_path())
        {
            std::error_code ec;
            std::filesystem::create_directories(outPath.parent_path(), ec);
        }

        vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        // PFGroup 顶点为 double*，此处按 double 写入
        points->SetDataTypeToDouble();
        grid->SetPoints(points);

        vtkSmartPointer<vtkIntArray> groupIdArr = vtkSmartPointer<vtkIntArray>::New();
        groupIdArr->SetName("group_id");
        grid->GetCellData()->AddArray(groupIdArr);

        vtkSmartPointer<vtkStringArray> groupNameArr = vtkSmartPointer<vtkStringArray>::New();
        groupNameArr->SetName("group_name");
        grid->GetCellData()->AddArray(groupNameArr);

        vtkSmartPointer<vtkStringArray> volumeNameArr = vtkSmartPointer<vtkStringArray>::New();
        volumeNameArr->SetName("volume_name");
        grid->GetCellData()->AddArray(volumeNameArr);

        // 收集 group 指针，避免重复（volume 内 groups 可能与顶层 groups 重叠）
        // group -> 所属体名（无所属体则为空）
        std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupsToExport;
        groupsToExport.reserve(static_cast<size_t>(data.getGroupSize()) + 64);
        std::set<PREPRO_BASE_NAMESPACE::PFGroup*> seenGroups;
        std::map<PREPRO_BASE_NAMESPACE::PFGroup*, std::string> groupToVolumeName;

        if (data.getVolumeSize() > 0)
        {
            auto** vols = data.getVolumes();
            for (unsigned int i = 0; i < data.getVolumeSize(); ++i)
            {
                auto* v = vols ? vols[i] : nullptr;
                if (!v) continue;
                const char* vnameC = v->getName();
                std::string vname = (vnameC && vnameC[0]) ? std::string(vnameC)
                    : ("Volume_" + std::to_string(v->getId()));
                auto** vgs = v->getGroups();
                for (unsigned int j = 0; j < v->getGroupSize(); ++j)
                {
                    auto* g = vgs ? vgs[j] : nullptr;
                    if (!g) continue;
                    groupToVolumeName[g] = vname;
                    if (seenGroups.insert(g).second)
                        groupsToExport.push_back(g);
                }
            }
        }

        if (data.getGroupSize() > 0)
        {
            auto** gs = data.getGroups();
            for (unsigned int i = 0; i < data.getGroupSize(); ++i)
            {
                if (gs && gs[i] && seenGroups.insert(gs[i]).second)
                    groupsToExport.push_back(gs[i]);
            }
        }

        vtkIdType pointOffset = 0;
        for (auto* group : groupsToExport)
        {
            if (!group) continue;
            const char* gnameC = group->getName();
            std::string gname = (gnameC && gnameC[0]) ? std::string(gnameC) : ("Group_" + std::to_string(group->getId()));
            int gid = static_cast<int>(group->getId());
            std::string vname;
            auto vit = groupToVolumeName.find(group);
            if (vit != groupToVolumeName.end())
                vname = vit->second;

            const size_t nPts = group->getVertexSize();
            double* vtx = group->getVertexes();
            if (nPts == 0 || !vtx) continue;

            // 追加点
            for (size_t pi = 0; pi < nPts; ++pi)
            {
                const size_t idx = 3 * pi;
                points->InsertNextPoint(vtx[idx], vtx[idx + 1], vtx[idx + 2]);
            }

            // 追加单元
            const size_t nElems = group->getElementSize();
            PREPRO_BASE_NAMESPACE::PFElement** elems = group->getElements();
            if (nElems == 0 || !elems)
            {
                pointOffset += static_cast<vtkIdType>(nPts);
                continue;
            }

            for (size_t ei = 0; ei < nElems; ++ei)
            {
                auto* e = elems[ei];
                if (!e) continue;
                const unsigned int nv = e->getVertexSize();
                unsigned int* vids = e->getVertexes();
                if (nv == 0 || !vids) continue;

                // 映射到全局点索引（group 内索引 + offset）
                std::vector<vtkIdType> cellIds;
                cellIds.reserve(nv);
                bool valid = true;
                for (unsigned int k = 0; k < nv; ++k)
                {
                    if (vids[k] >= nPts)
                    {
                        valid = false;
                        break;
                    }
                    cellIds.push_back(pointOffset + static_cast<vtkIdType>(vids[k]));
                }
                if (!valid) continue;

                int vtkCellType = VTK_POLYGON;
                if (nv == 1) vtkCellType = VTK_VERTEX;
                else if (nv == 2) vtkCellType = VTK_LINE;
                else if (nv == 3) vtkCellType = VTK_TRIANGLE;
                else if (nv == 4) vtkCellType = VTK_QUAD;
                else vtkCellType = VTK_POLYGON;

                grid->InsertNextCell(vtkCellType, static_cast<vtkIdType>(cellIds.size()), cellIds.data());
                groupIdArr->InsertNextValue(gid);
                groupNameArr->InsertNextValue(gname.c_str());
                volumeNameArr->InsertNextValue(vname.c_str());
            }

            pointOffset += static_cast<vtkIdType>(nPts);
        }

        vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
        writer->SetFileName(outPath.string().c_str());
        writer->SetInputData(grid);
        writer->SetDataModeToBinary();
        if (writer->Write() == 0)
        {
            if (errorOut) *errorOut = "VTK 写文件失败: " + outPath.string();
            return false;
        }
        return true;
    }
    catch (const std::exception& e)
    {
        if (errorOut) *errorOut = std::string("导出 vtu 异常: ") + e.what();
        return false;
    }
}

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
    m_sharedAPI->registerCommand("ExportMeshToVtu", [this](const json& p) { return handleExportMeshToVtu(p); });
}

VolumeProcessor* ModelProcessingServer::getSessionForDataQuery(const std::string& sessionId)
{
    return getSession(sessionId);
}

GeometryAPI* ModelProcessingServer::tryGetGeometryAPIForPath(const std::string& ppcfPath)
{
    if (ppcfPath.empty()) return nullptr;
    std::filesystem::path req(ppcfPath);
    std::string meshSessionIdCopy;
    bool hitMeshSessionPath = false;
    bool hitMeshApiPath = false;

    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        if (m_meshGeometryAPI && !m_meshOpenDocumentPath.empty() && std::filesystem::path(m_meshOpenDocumentPath) == req)
        {
            hitMeshApiPath = true;
        }
        if (!m_meshSessionId.empty() && !m_meshSessionProjectPath.empty() && std::filesystem::path(m_meshSessionProjectPath) == req)
        {
            hitMeshSessionPath = true;
            meshSessionIdCopy = m_meshSessionId;
        }
    }

    if (hitMeshApiPath)
    {
        GeometryAPI* api = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_meshSessionMutex);
            api = m_meshGeometryAPI.get();
        }
        if (api && geometryApiHasReadableMeshData(api))
            return api;

        // 路径命中缓存但文档不含可读网格：openDocument 优先，必要时回退 importMesh 重新加载并刷新缓存
        std::string err;
        std::unique_ptr<GeometryAPI> reloaded;
        if (!reloadMeshGeometryApiFromPpcf(ppcfPath, reloaded, &err))
        {
            if (m_logger) m_logger->logOutputLine("[tryGetGeometryAPIForPath] 警告: 缓存命中但无网格数据，重载失败: " + err);
            return nullptr;
        }
        {
            std::lock_guard<std::mutex> lock(m_meshSessionMutex);
            m_meshGeometryAPI = std::move(reloaded);
            m_meshOpenDocumentPath = ppcfPath;
            // 避免 session 路径与独立 mesh API 指向同一文件时出现“双份文档”不一致
            m_meshSessionId.clear();
            m_meshSessionProjectPath.clear();
            return m_meshGeometryAPI.get();
        }
    }

    if (hitMeshSessionPath)
    {
        VolumeProcessor* p = getSession(meshSessionIdCopy);
        GeometryAPI* api = p ? p->getGeometryAPI() : nullptr;
        if (api && geometryApiHasReadableMeshData(api))
            return api;

        std::string err;
        std::unique_ptr<GeometryAPI> reloaded;
        if (!reloadMeshGeometryApiFromPpcf(ppcfPath, reloaded, &err))
        {
            if (m_logger) m_logger->logOutputLine("[tryGetGeometryAPIForPath] 警告: session 缓存命中但无网格数据，重载失败: " + err);
            return nullptr;
        }
        {
            std::lock_guard<std::mutex> lock(m_meshSessionMutex);
            m_meshGeometryAPI = std::move(reloaded);
            m_meshOpenDocumentPath = ppcfPath;
            m_meshSessionId.clear();
            m_meshSessionProjectPath.clear();
            return m_meshGeometryAPI.get();
        }
    }

    return nullptr;
}

VolumeProcessor* ModelProcessingServer::tryGetVolumeProcessorForPath(const std::string& ppcfPath)
{
    if (ppcfPath.empty()) return nullptr;
    std::string meshSessionIdCopy;
    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        std::filesystem::path req(ppcfPath);
        if (!m_meshSessionId.empty() && !m_meshSessionProjectPath.empty() && std::filesystem::path(m_meshSessionProjectPath) == req)
            meshSessionIdCopy = m_meshSessionId;
    }
    if (meshSessionIdCopy.empty()) return nullptr;

    VolumeProcessor* p = getSession(meshSessionIdCopy);
    GeometryAPI* api = p ? p->getGeometryAPI() : nullptr;
    if (api && geometryApiHasReadableMeshData(api))
        return p;

    // session 命中但网格不可读：同上，刷新为 importMesh 的独立会话（VolumeProcessor 映射会暂时失效）
    std::string err;
    std::unique_ptr<GeometryAPI> reloaded;
    if (!reloadMeshGeometryApiFromPpcf(ppcfPath, reloaded, &err))
    {
        if (m_logger) m_logger->logOutputLine("[tryGetVolumeProcessorForPath] 警告: session 缓存命中但无网格数据，重载失败: " + err);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        m_meshGeometryAPI = std::move(reloaded);
        m_meshOpenDocumentPath = ppcfPath;
        m_meshSessionId.clear();
        m_meshSessionProjectPath.clear();
    }
    return nullptr;
}

GeometryAPI* ModelProcessingServer::tryGetGeometryAPIForMesh()
{
    GeometryAPI* api = nullptr;
    std::string meshSessionIdCopy;
    std::string meshSessionPpcfCopy;
    {
        std::lock_guard<std::mutex> lock(m_meshSessionMutex);
        if (m_meshGeometryAPI && !m_meshOpenDocumentPath.empty())
            api = m_meshGeometryAPI.get();
        if (!m_meshSessionId.empty() && !m_meshSessionProjectPath.empty())
        {
            meshSessionIdCopy = m_meshSessionId;
            meshSessionPpcfCopy = m_meshSessionProjectPath;
        }
    }

    if (api && geometryApiHasReadableMeshData(api))
        return api;

    if (!meshSessionIdCopy.empty())
    {
        VolumeProcessor* p = getSession(meshSessionIdCopy);
        GeometryAPI* sapi = p ? p->getGeometryAPI() : nullptr;
        if (sapi && geometryApiHasReadableMeshData(sapi))
            return sapi;
    }

    // 复用会话但读不到网格：若知道 ppcf 路径，则 importMesh 刷新缓存
    if (!meshSessionPpcfCopy.empty() && std::filesystem::exists(meshSessionPpcfCopy))
    {
        std::string err;
        std::unique_ptr<GeometryAPI> reloaded;
        if (reloadMeshGeometryApiFromPpcf(meshSessionPpcfCopy, reloaded, &err))
        {
            std::lock_guard<std::mutex> lock(m_meshSessionMutex);
            m_meshGeometryAPI = std::move(reloaded);
            m_meshOpenDocumentPath = meshSessionPpcfCopy;
            m_meshSessionId.clear();
            m_meshSessionProjectPath.clear();
            return m_meshGeometryAPI.get();
        }
        if (m_logger) m_logger->logOutputLine("[tryGetGeometryAPIForMesh] 警告: 无可读网格数据，重载失败: " + err);
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
    // 快速修复容差：未传时保持历史默认 1e-5（与原先 quickRepair 硬编码一致）
    options.repairTolerance = params.value("repairTolerance", 1e-5);

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

    bool verboseLog = params.value("verboseLog", false);
    // 可选：包围盒匹配容差（默认沿用 processor 默认配置 repairTolerance）
    double tolerance = params.value("tolerance", processor->getCurrentProcessOptions().repairTolerance);
    if (m_logger)
        m_logger->logOutputLine("[ExecuteGeometryMatching] tolerance=" + std::to_string(tolerance) + ", verboseLog=" + std::string(verboseLog ? "true" : "false"));

    if (!processor->executeGeometryMatching(&model, tolerance, verboseLog))
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
    GeometryAPI* meshSessionApi = tryGetGeometryAPIForMesh();

    if (!meshSessionApi && !ppcfPath.empty())
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
        if (!meshSessionApi)
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
            // 会话优先：网格划分成功后直接复用内存中的网格会话，避免再次按路径加载导致拿到几何-only 文档。
            GeometryAPI* apiToUse = tryGetGeometryAPIForMesh();
            std::unique_ptr<GeometryAPI> geometryAPIOwned;
            if (apiToUse)
            {
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 复用 GeometryAPI（网格会话优先）");
            }
            else if (!ppcfPath.empty())
            {
                // 无可用会话时，按路径兜底加载
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
                    if (!geometryAPIOwned->importMesh(ppcfPath) && !geometryAPIOwned->openDocument(ppcfPath))
                    {
                        if (m_logger) m_logger->logOutputLine("[ShowMesh] importMesh/openDocument 均失败: " + geometryAPIOwned->getLastError());
                        return;
                    }
                    apiToUse = geometryAPIOwned.get();
                }
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 无网格会话，已按路径加载");
            }
            else
            {
                if (m_logger) m_logger->logOutputLine("[ShowMesh] 失败: 无可用网格会话，且未提供 ppcfPath");
                return;
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
            bool usedGeometryFallback = false;
            if (!meshProcessor.getMeshData(data))
            {
                // 额外记录一次底层网格统计，便于区分“网格生成失败”与“网格已生成但当前文档未暴露到 getAllData”。
                auto* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
                if (pfMesh)
                {
                    PREPRO_BASE_NAMESPACE::PFData meshRawData;
                    PREPRO_BASE_NAMESPACE::PFStatus meshStatus = pfMesh->getAllData(meshRawData);
                    if (m_logger)
                    {
                        if (meshStatus == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                        {
                            m_logger->logOutputLine("[ShowMesh] 网格数据检查: getAllData=EOkay, groupSize=" +
                                std::to_string(meshRawData.getGroupSize()) + ", volumeSize=" + std::to_string(meshRawData.getVolumeSize()));
                        }
                        else
                        {
                            m_logger->logOutputLine("[ShowMesh] 网格数据检查: getAllData 非 EOkay, status=" + std::to_string(static_cast<int>(meshStatus)));
                        }
                    }
                }

                auto* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
                if (!pfGeometry || pfGeometry->getAllData(data) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                {
                    if (m_logger) m_logger->logOutputLine("[ShowMesh] 无法获取网格/几何数据");
                    return;
                }
                usedGeometryFallback = true;
            }
            if (usedGeometryFallback && m_logger)
                m_logger->logOutputLine("[ShowMesh] 警告: 未读取到网格数据，已回退到几何数据进行显示");
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

    // 删除后仅刷新顶层数据，不 findVolumes（findVolumes 会缝合封闭域，可能错误围出新体）
    processor->onVolumeDeletedByName(volumeName);

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
    const auto meshGenStart = std::chrono::high_resolution_clock::now();
    bool ok = geometryAPIIn
        ? ModelProcessingServer::executeMeshGenerationFromJson(jsonPath, &err, ppcfPathToUse, m_logger, geometryAPIIn, nullptr, nullptr)
        : ModelProcessingServer::executeMeshGenerationFromJson(jsonPath, &err, ppcfPathToUse, m_logger, nullptr, &m_meshGeometryAPI, &m_meshOpenDocumentPath);
    const auto meshGenEnd = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> meshGenElapsed = meshGenEnd - meshGenStart;

    if (ok)
    {
        response["success"] = true;
        response["message"] = "网格划分完成";
        response["ppcfPath"] = ppcfPathToUse;
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

    if (m_logger)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << "[ExecuteMeshGeneration] 网格划分总耗时: " << meshGenElapsed.count() << " 秒";
        m_logger->logOutputLine(oss.str());
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

    // 保护：该接口语义是“保存包含网格的 ppcf”。
    // 若源文件当前不含网格，直接保存会产生“成功但无网格”的结果，容易误用。
    {
        auto* pfDocument = geometryAPI.getDocument();
        MeshProcessor meshProcessor(pfDocument);
        bool hasMesh = meshProcessor.initialize() && meshProcessor.hasMeshData();
        if (!hasMesh)
        {
            if (errorOut)
            {
                *errorOut = "源 ppcf 不包含网格数据（请先执行 ExecuteMeshGeneration 并使用其返回的 ppcfPath，或先调用 SavePpcf 保存当前网格会话）";
            }
            return false;
        }
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
        bool hasMeshData = false;
        {
            MeshProcessor meshProcessor(apiToSave->getDocument());
            hasMeshData = meshProcessor.initialize() && meshProcessor.hasMeshData();
        }
        if (m_logger)
            m_logger->logOutputLine(std::string("[SavePpcf] 保存前文档检查: hasMeshData=") + (hasMeshData ? "true" : "false"));

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
    // 会话优先：若已有网格会话，直接使用，避免每次按路径重载。
    GeometryAPI* apiToUse = tryGetGeometryAPIForMesh();
    std::unique_ptr<GeometryAPI> geometryAPIOwned;

    if (!apiToUse && ppcfPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 ppcfPath 参数，且当前无可复用网格会话（请先执行 ExecuteMeshGeneration 或传入包含网格的 ppcfPath）";
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: " + response["error"].get<std::string>());
        return response;
    }

    if (!apiToUse && !ppcfPath.empty())
    {
        if (!std::filesystem::exists(ppcfPath))
        {
            response["success"] = false;
            response["error"] = "ppcf 文件不存在: " + ppcfPath;
            if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 失败: " + response["error"].get<std::string>());
            return response;
        }

        apiToUse = tryGetGeometryAPIForPath(ppcfPath);
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
        if (m_logger) m_logger->logOutputLine("[GetMeshQuality] 无网格会话，已按路径加载");
    }
    else if (apiToUse && m_logger)
    {
        m_logger->logOutputLine("[GetMeshQuality] 复用 GeometryAPI（网格会话优先）");
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
        PREPRO_BASE_NAMESPACE::PFData data;
        unsigned int volumeCount = 0;
        unsigned int groupCount = 0;
        if (pfMesh->getAllData(data) == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            volumeCount = data.getVolumeSize();
            groupCount = data.getGroupSize();
        }

        response["success"] = false;
        response["error"] = "无网格质量数据（可能打开了几何-only ppcf，或该网格尚无可用质量统计）";
        response["meshVolumeCount"] = volumeCount;
        response["meshGroupCount"] = groupCount;
        if (!ppcfPath.empty()) response["ppcfPath"] = ppcfPath;
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
    auto geometryAPI = std::make_unique<GeometryAPI>();
    if (!geometryAPI->initialize())
    {
        response["success"] = false;
        response["error"] = "GeometryAPI 初始化失败";
        if (m_logger) m_logger->logOutputLine("[ImportPpcf] 失败: GeometryAPI 初始化失败");
        return response;
    }

    bool useOpenDocument = (importMode != "mesh");
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

json ModelProcessingServer::handleExportMeshToVtu(const json& params)
{
    json response;
    std::string vtuPath = params.value("vtuPath", "");
    std::string ppcfPath = params.value("ppcfPath", "");

    if (vtuPath.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 vtuPath 参数（导出的 .vtu 文件路径）";
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
        return response;
    }

    // 会话优先：导出当前网格会话，保证与刚划分完成的数据一致。
    GeometryAPI* apiToUse = tryGetGeometryAPIForMesh();
    std::unique_ptr<GeometryAPI> geometryAPIOwned;

    if (!apiToUse && !ppcfPath.empty())
    {
        if (!std::filesystem::exists(ppcfPath))
        {
            response["success"] = false;
            response["error"] = "ppcf 文件不存在: " + ppcfPath;
            if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
            return response;
        }

        apiToUse = tryGetGeometryAPIForPath(ppcfPath);
        if (!apiToUse)
        {
            geometryAPIOwned = std::make_unique<GeometryAPI>();
            if (!geometryAPIOwned->initialize())
            {
                response["success"] = false;
                response["error"] = "GeometryAPI 初始化失败";
                if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: GeometryAPI 初始化失败");
                return response;
            }
            // 导出优先 importMesh：更贴近“网格数据”加载路径；失败再回退 openDocument
            if (!geometryAPIOwned->importMesh(ppcfPath) && !geometryAPIOwned->openDocument(ppcfPath))
            {
                response["success"] = false;
                response["error"] = std::string("importMesh/openDocument 均失败: ") + geometryAPIOwned->getLastError();
                if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
                return response;
            }
            apiToUse = geometryAPIOwned.get();
        }
        else if (m_logger)
        {
            m_logger->logOutputLine("[ExportMeshToVtu] 复用 GeometryAPI（ppcf 已打开）");
        }
    }
    else if (apiToUse)
    {
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 复用 GeometryAPI（网格会话优先）");
    }
    else
    {
        response["success"] = false;
        response["error"] = "未传入 ppcfPath 且无缓存的网格会话（请先执行 ExecuteMeshGeneration 或 ImportPpcf）";
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
        return response;
    }

    auto* pfDocument = apiToUse->getDocument();
    if (!pfDocument)
    {
        response["success"] = false;
        response["error"] = "无法获取文档";
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: 无法获取文档");
        return response;
    }

    MeshProcessor meshProcessor(pfDocument);
    if (!meshProcessor.initialize())
    {
        response["success"] = false;
        response["error"] = "MeshProcessor 初始化失败";
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: MeshProcessor 初始化失败");
        return response;
    }

    PREPRO_BASE_NAMESPACE::PFData meshData;
    if (!meshProcessor.getMeshData(meshData))
    {
        // 若复用了缓存的 GeometryAPI，但文档未刷新到含网格状态：对传入 ppcfPath 的场景用全新 GeometryAPI 再 importMesh/openDocument 一次
        if (!ppcfPath.empty() && !geometryAPIOwned)
        {
            auto retryAPI = std::make_unique<GeometryAPI>();
            if (retryAPI->initialize() && retryAPI->getDocument())
            {
                bool opened = retryAPI->importMesh(ppcfPath) || retryAPI->openDocument(ppcfPath);
                if (opened)
                {
                    MeshProcessor retryMeshProcessor(retryAPI->getDocument());
                    if (retryMeshProcessor.initialize() && retryMeshProcessor.getMeshData(meshData))
                    {
                        geometryAPIOwned = std::move(retryAPI);
                        apiToUse = geometryAPIOwned.get();
                    }
                }
            }
        }
    }
    if (meshData.getGroupSize() == 0 && meshData.getVolumeSize() == 0)
    {
        response["success"] = false;
        response["error"] = "未读取到网格数据（请确认 ppcf 包含网格，或先执行 ExecuteMeshGeneration）";
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
        return response;
    }

    std::string err;
    if (!exportMeshPfDataToVtu(meshData, vtuPath, &err))
    {
        response["success"] = false;
        response["error"] = err.empty() ? "导出 vtu 失败" : err;
        if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 失败: " + response["error"].get<std::string>());
        return response;
    }

    response["success"] = true;
    response["message"] = "已导出 vtu: " + vtuPath;
    response["vtuPath"] = vtuPath;
    if (!ppcfPath.empty()) response["ppcfPath"] = ppcfPath;
    if (m_logger) m_logger->logOutputLine("[ExportMeshToVtu] 成功: " + vtuPath);
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
                            const auto& fzs = bl["FluidZoneSet"];
                            if (fzs.is_array())
                            {
                                for (const auto& item : fzs)
                                {
                                    if (item.is_string())
                                    {
                                        std::string name = item.get<std::string>();
                                        if (!name.empty())
                                            meshParams.fluidZoneSetNames.push_back(name);
                                    }
                                }
                            }
                            else if (fzs.is_string())
                            {
                                std::string name = fzs.get<std::string>();
                                if (!name.empty())
                                    meshParams.fluidZoneSetNames.push_back(name);
                            }
                            if (blParams.isBoundaryLayers && !meshParams.fluidZoneSetNames.empty())
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

        // 划分成功后，从 SDK getter 读回实际生效参数并输出
        if (logger) logger->logOutputLine("[ExecuteMeshGeneration] 读取 SDK 实际生效网格参数(get)...");
        else std::cout << "[ExecuteMeshGeneration] 读取 SDK 实际生效网格参数(get)..." << std::endl;
        meshProcessor.printEffectiveParameters();

        {
            std::ostringstream oss;
            oss << "[ExecuteMeshGeneration] 网格划分完成，本次使用的网格参数汇总:\n";
            oss << "  [Global]\n";
            oss << "    MinSize(m)=" << meshParams.minSize << "\n";
            oss << "    MaxSize(m)=" << meshParams.maxSize << "\n";
            oss << "    GrowthRate=" << meshParams.growthRate << "\n";
            oss << "    CurvatureNormalAngle=" << meshParams.curvatureNormalAngle << "\n";
            oss << "    ProximityEnabled=" << (meshParams.proximityEnabled ? "true" : "false") << "\n";
            oss << "    CellsPerGap=" << meshParams.cellsPerGap << "\n";
            oss << "    MinimumGapSize(m)=" << meshParams.minimumGapSize << "\n";
            oss << "    AllTrianglesEnabled=" << (meshParams.allTrianglesEnabled ? "true" : "false") << "\n";
            oss << "    MeshQualityOptimizationEnabled=" << (meshParams.meshQualityOptimizationEnabled ? "true" : "false") << "\n";
            oss << "    InflationMinimumQuality=" << meshParams.inflationMinimumQuality << "\n";
            oss << "    InflationSeparatingAngle=" << meshParams.inflationSeparatingAngle << "\n";
            oss << "    InflationMaximumHeightBaseRatio=" << meshParams.inflationMaximumHeightBaseRatio << "\n";

            oss << "  [BoundaryLayer]\n";
            if (meshParams.boundaryLayerParams.has_value() && !meshParams.fluidZoneSetNames.empty())
            {
                const auto& bl = meshParams.boundaryLayerParams.value();
                oss << "    Enabled=" << (bl.isBoundaryLayers ? "true" : "false") << "\n";
                oss << "    FluidZoneSet=[";
                bool firstZone = true;
                for (const auto& zoneName : meshParams.fluidZoneSetNames)
                {
                    if (!firstZone) oss << ", ";
                    oss << zoneName;
                    firstZone = false;
                }
                oss << "]\n";
                oss << "    FirstLayerHeight(m)=" << bl.firstLayerHeight << "\n";
                oss << "    GrowthRate=" << bl.growthRate << "\n";
                oss << "    LayersNumber=" << bl.layersNumber << "\n";
                oss << "    ExcludedBoundaryNamesCount=" << meshParams.excludedBoundaryNames.size() << "\n";
                if (!meshParams.excludedBoundaryNames.empty())
                {
                    oss << "    ExcludedBoundaryNames=";
                    bool first = true;
                    for (const auto& n : meshParams.excludedBoundaryNames)
                    {
                        if (!first) oss << ", ";
                        oss << n;
                        first = false;
                    }
                    oss << "\n";
                }
            }
            else
            {
                oss << "    (not configured)\n";
            }

            oss << "  [LocalMesh] Count=" << localMeshItems.size() << "\n";
            for (size_t i = 0; i < localMeshItems.size(); ++i)
            {
                const auto& it = localMeshItems[i];
                oss << "    [" << (i + 1) << "] "
                    << "Name=" << it.Name.value_or("")
                    << ", RefinementSet=" << it.RefinementSet.value_or("")
                    << ", MinMeshSize(mm)=" << it.MinMeshSize.value_or(0.0)
                    << ", MaxMeshSize(mm)=" << it.MaxMeshSize.value_or(0.0)
                    << ", GrowthRate=" << it.GrowthRate.value_or(0.0)
                    << ", NormalAngle=" << it.NormalAngle.value_or(0.0)
                    << "\n";
            }

            if (logger) logger->logOutputLine(oss.str());
            else std::cout << oss.str() << std::endl;
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
