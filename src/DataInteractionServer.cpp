#include "DataInteractionServer.h"
#include "ModelProcessingServer.h"
#include "VolumeProcessor.h"
#include "GeometryAPI.h"
#include "MeshProcessor.h"
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

DataInteractionServer::DataInteractionServer(SocketCommandAPI* sharedAPI, ModelProcessingServer* modelServer)
    : m_sharedAPI(sharedAPI)
    , m_modelServer(modelServer)
{
    if (!m_sharedAPI) return;

    m_sharedAPI->registerCommand("GetVolumeListNames", [this](const json& p) { return handleGetVolumeListNames(p); });
    m_sharedAPI->registerCommand("GetFaceGroupNamesByVolume", [this](const json& p) { return handleGetFaceGroupNamesByVolume(p); });
    m_sharedAPI->registerCommand("RefreshSessionData", [this](const json& p) { return handleRefreshSessionData(p); });
}

DataInteractionServer::~DataInteractionServer()
{
}

static void applyVolumeRenameDisplayNames(std::vector<std::string>& names,
    const std::vector<std::pair<std::string, std::string>>& renames)
{
    if (renames.empty() || names.empty())
        return;
    std::unordered_map<std::string, std::string> oldToNew;
    oldToNew.reserve(renames.size() * 2 + 1);
    for (const auto& p : renames)
        oldToNew[p.first] = p.second;
    for (auto& n : names)
    {
        auto it = oldToNew.find(n);
        if (it != oldToNew.end())
            n = it->second;
    }
}

/**
 * 部分情况下枚举仍可能出现“已删除”占位体，名称形如 "GPU_S-REMOVED-1.1"。
 * 不加入对外体列表（与 SavePpcf 是否仍含内部节点无关）。
 */
static bool isSdkDeletedVolumePlaceholder(const std::string& n)
{
    return n.find("_S-REMOVED-") != std::string::npos;
}

/** SDK 枚举的体名可能仍为旧名；客户端传新名时需映射回旧名再与 vol->getName() 比较 */
static std::string sdkVolumeNameForLookup(const std::string& requested,
    const std::vector<std::pair<std::string, std::string>>& renames)
{
    if (renames.empty())
        return requested;
    for (const auto& p : renames)
    {
        if (p.second == requested)
            return p.first;
    }
    return requested;
}

static bool getPFDataFromSession(VolumeProcessor* processor, PREPRO_BASE_NAMESPACE::PFData& data)
{
    if (!processor) return false;
    GeometryAPI* api = processor->getGeometryAPI();
    if (!api) return false;

    // 始终从几何层获取数据：几何识别匹配（analyzeVolumes）在几何层重命名体，
    // 若从网格层（pfMesh）获取，可能得到重命名前的旧名称
    return api->getAllData(data);
}

static bool getPFDataFromPpcf(const std::string& ppcfPath, PREPRO_BASE_NAMESPACE::PFData& data, ModelProcessingServer* modelServer)
{
    // 与 getPFDataFromSession 一致：几何识别匹配在几何层 renameVolume，网格层 PFData 可能仍是重命名前的快照；
    // 若先取 mesh->getAllData，GetVolumeListNames 会长期返回旧体名，直到重新划分网格。
    auto fillFromDocument = [](PREPRO_BASE_NAMESPACE::PFDocument* pfDocument, PREPRO_BASE_NAMESPACE::PFData& out) -> bool {
        if (!pfDocument) return false;
        auto* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
        if (pfGeometry && pfGeometry->getAllData(out) == PREPRO_BASE_NAMESPACE::PFStatus::EOkay &&
            out.getVolumeSize() > 0)
            return true;
        MeshProcessor meshProcessor(pfDocument);
        if (meshProcessor.initialize() && meshProcessor.getMeshData(out))
            return true;
        if (pfGeometry && pfGeometry->getAllData(out) == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            return true;
        return false;
    };

    GeometryAPI* apiReuse = modelServer ? modelServer->tryGetGeometryAPIForPath(ppcfPath) : nullptr;
    if (apiReuse)
    {
        return fillFromDocument(apiReuse->getDocument(), data);
    }
    GeometryAPI geometryAPI;
    if (!geometryAPI.initialize() || !geometryAPI.importMesh(ppcfPath))
        return false;

    return fillFromDocument(geometryAPI.getDocument(), data);
}

json DataInteractionServer::handleGetVolumeListNames(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    std::string ppcfPath = params.value("ppcfPath", "");

    PREPRO_BASE_NAMESPACE::PFData data;
    bool hasData = false;
    VolumeProcessor* renameProcessor = nullptr;

    if (m_modelServer && !sessionId.empty())
    {
        VolumeProcessor* processor = m_modelServer->getSessionForDataQuery(sessionId);
        hasData = getPFDataFromSession(processor, data);
        if (hasData)
            renameProcessor = processor;
        if (!hasData && processor)
        {
            response["success"] = false;
            response["error"] = "无法从会话顶层数据获取";
            return response;
        }
    }

    if (!hasData && !ppcfPath.empty() && std::filesystem::exists(ppcfPath))
    {
        hasData = getPFDataFromPpcf(ppcfPath, data, m_modelServer);
        if (hasData && m_modelServer && !renameProcessor)
            renameProcessor = m_modelServer->tryGetVolumeProcessorForPath(ppcfPath);
    }

    if (!hasData)
    {
        response["success"] = false;
        response["error"] = sessionId.empty() && ppcfPath.empty()
            ? "缺少 sessionId 或 ppcfPath 参数"
            : "无法获取数据（请先 ImportGeometryModel 或提供有效 ppcfPath）";
        return response;
    }

    std::vector<std::string> volumeNames;
    if (renameProcessor)
    {
        if (!renameProcessor->getVolumeListDisplayNames(volumeNames))
        {
            response["success"] = false;
            response["error"] = "无法获取体名称列表";
            return response;
        }
    }
    else
    {
        int volumeSize = static_cast<int>(data.getVolumeSize());
        PREPRO_BASE_NAMESPACE::PFVolume** volumes = data.getVolumes();
        if (volumes)
        {
            for (int i = 0; i < volumeSize; i++)
            {
                PREPRO_BASE_NAMESPACE::PFVolume* vol = volumes[i];
                if (vol)
                {
                    char* name = vol->getName();
                    if (name && name[0] != '\0')
                    {
                        std::string raw(name);
                        if (!isSdkDeletedVolumePlaceholder(raw))
                            volumeNames.push_back(std::move(raw));
                    }
                }
            }
        }
    }

    response["success"] = true;
    response["volumeNames"] = volumeNames;
    if (!sessionId.empty()) response["sessionId"] = sessionId;
    return response;
}

json DataInteractionServer::handleGetFaceGroupNamesByVolume(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    std::string ppcfPath = params.value("ppcfPath", "");
    std::string volumeName = params.value("volumeName", "");

    if (volumeName.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 volumeName 参数";
        return response;
    }

    PREPRO_BASE_NAMESPACE::PFData data;
    bool hasData = false;
    VolumeProcessor* renameProcessor = nullptr;

    if (m_modelServer && !sessionId.empty())
    {
        VolumeProcessor* processor = m_modelServer->getSessionForDataQuery(sessionId);
        hasData = getPFDataFromSession(processor, data);
        if (hasData)
            renameProcessor = processor;
    }

    if (!hasData && !ppcfPath.empty() && std::filesystem::exists(ppcfPath))
    {
        hasData = getPFDataFromPpcf(ppcfPath, data, m_modelServer);
        if (hasData && m_modelServer && !renameProcessor)
            renameProcessor = m_modelServer->tryGetVolumeProcessorForPath(ppcfPath);
    }

    if (!hasData)
    {
        response["success"] = false;
        response["error"] = sessionId.empty() && ppcfPath.empty()
            ? "缺少 sessionId 或 ppcfPath 参数"
            : "无法获取数据";
        return response;
    }

    std::string volumeNameForSdk = volumeName;
    if (renameProcessor)
        volumeNameForSdk = sdkVolumeNameForLookup(volumeName, renameProcessor->getSessionVolumeRenames());

    std::vector<std::string> faceGroupNames;
    int volumeSize = static_cast<int>(data.getVolumeSize());
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = data.getVolumes();
    if (volumes)
    {
        for (int i = 0; i < volumeSize; i++)
        {
            PREPRO_BASE_NAMESPACE::PFVolume* vol = volumes[i];
            if (!vol) continue;

            char* volName = vol->getName();
            if (!volName)
                continue;
            const std::string volNameStr(volName);
            if (volNameStr != volumeNameForSdk && volNameStr != volumeName)
                continue;

            int groupSize = static_cast<int>(vol->getGroupSize());
            PREPRO_BASE_NAMESPACE::PFGroup** groups = vol->getGroups();
            if (groups)
            {
                for (int j = 0; j < groupSize; j++)
                {
                    PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
                    if (group)
                    {
                        char* groupName = group->getName();
                        if (groupName && groupName[0] != '\0')
                        {
                            std::string nameStr(groupName);
                            if (nameStr.find("node") == std::string::npos &&
                                nameStr.find("edge") == std::string::npos &&
                                (nameStr.empty() || nameStr[0] != '.'))
                            {
                                faceGroupNames.push_back(nameStr);
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    response["success"] = true;
    response["volumeName"] = volumeName;
    response["faceGroupNames"] = faceGroupNames;
    if (!sessionId.empty()) response["sessionId"] = sessionId;
    return response;
}

json DataInteractionServer::handleRefreshSessionData(const json& params)
{
    json response;
    std::string sessionId = params.value("sessionId", "");
    if (sessionId.empty())
    {
        response["success"] = false;
        response["error"] = "缺少 sessionId 参数";
        return response;
    }

    if (!m_modelServer)
    {
        response["success"] = false;
        response["error"] = "ModelProcessingServer 未配置";
        return response;
    }

    VolumeProcessor* processor = m_modelServer->getSessionForDataQuery(sessionId);
    if (!processor)
    {
        response["success"] = false;
        response["error"] = "会话不存在或已过期: " + sessionId;
        return response;
    }

    if (processor->refreshGeometryData())
    {
        response["success"] = true;
        response["message"] = "顶层数据已刷新";
    }
    else
    {
        response["success"] = false;
        response["error"] = processor->getLastError();
    }
    return response;
}
