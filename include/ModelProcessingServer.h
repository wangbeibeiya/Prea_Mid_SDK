#pragma once

#include "SocketCommandAPI.h"
#include "json.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

using json = nlohmann::json;

class VolumeProcessor;
class GeometryAPI;
class Logger;

/**
 * @brief 模型处理服务类
 *
 * 通过 Socket 接收命令，执行几何匹配和网格划分。
 * 与 MeshVisualizationServer、VolumeVisualizationServer 共用同一 Socket。
 *
 * 几何流程拆分为三步（后续步骤复用导入后的顶层数据）：
 * - ImportGeometryModel: 1. 导入几何模型，返回 sessionId
 * - ExecuteGeometryProcessing: 2. 几何处理（quickRepair + findVolumes），需 sessionId
 * - ExecuteGeometryMatching: 3. 几何识别匹配（analyzeVolumes + 保存 ppcf），需 sessionId
 * - ExecuteMeshGeneration: 网格划分（加载几何 ppcf、生成网格、保存）
 */
class ModelProcessingServer
{
public:
    /**
     * @param sharedAPI 共享的 Socket 命令 API
     * @param logger 应用级日志（程序启动时创建，导入失败、JSON 失败等均记录）
     */
    explicit ModelProcessingServer(SocketCommandAPI* sharedAPI, Logger* logger = nullptr);
    ~ModelProcessingServer();

    /**
     * @brief 从 JSON 路径执行完整几何匹配（三步合一，供 main 等调用）
     * @param jsonPath JSON 项目文件路径
     * @param errorOut 失败时输出错误信息
     * @return 是否成功
     */
    static bool executeGeometryMatchingFromJson(const std::string& jsonPath, std::string* errorOut = nullptr);

    /**
     * @brief 从 JSON 路径执行网格划分（供 main 等调用，不依赖 Socket）
     * @param jsonPath JSON 项目文件路径
     * @param errorOut 失败时输出错误信息
     * @param ppcfPath 可选，几何 ppcf 文件路径；为空时从 json 的 WorkingDirectory/ProjectName 推导
     * @param logger 可选，日志对象；传入时网格划分的详细过程会输出到日志
     * @param geometryAPIIn 可选，复用几何 session 的 GeometryAPI，跳过 init + openDocument
     * @param meshSessionOut 可选，成功时保留 GeometryAPI 供 SaveMeshPpcf 复用（仅当 geometryAPIIn 为空时）
     * @param meshSessionPathOut 可选，与 meshSessionOut 配套，记录当前打开的文档路径
     * @return 是否成功
     */
    static bool executeMeshGenerationFromJson(const std::string& jsonPath, std::string* errorOut = nullptr, const std::string& ppcfPath = "", Logger* logger = nullptr,
        GeometryAPI* geometryAPIIn = nullptr, std::unique_ptr<GeometryAPI>* meshSessionOut = nullptr, std::string* meshSessionPathOut = nullptr);

    /**
     * @brief 保存包含几何和网格的 ppcf 文件（从已打开的文档或从文件）
     * @param sourcePpcfPath 源 ppcf 文件路径（需已包含几何和网格）
     * @param savePath 保存路径；为空时保存到 sourcePpcfPath（覆盖）
     * @param errorOut 失败时输出错误信息
     * @return 是否成功
     */
    static bool saveMeshPpcfToPath(const std::string& sourcePpcfPath, const std::string& savePath, std::string* errorOut = nullptr);

    /**
     * @brief 获取会话中的 VolumeProcessor（供 DataInteractionServer 从顶层数据查询）
     * @param sessionId 会话 ID
     * @return VolumeProcessor 指针，不存在返回 nullptr
     */
    VolumeProcessor* getSessionForDataQuery(const std::string& sessionId);

    /**
     * @brief 尝试获取已缓存的 GeometryAPI（供 ShowMesh、GetVolumeListNames 等复用）
     * @param ppcfPath ppcf 文件路径
     * @return GeometryAPI 指针，无缓存返回 nullptr；调用方需快速使用，勿长时间持有
     */
    GeometryAPI* tryGetGeometryAPIForPath(const std::string& ppcfPath);

    /**
     * @brief 获取当前网格会话的 GeometryAPI（无路径时复用 ExecuteMeshGeneration 后的缓存）
     * @return GeometryAPI 指针，无缓存返回 nullptr
     */
    GeometryAPI* tryGetGeometryAPIForMesh();

private:
    SocketCommandAPI* m_sharedAPI = nullptr;
    Logger* m_logger = nullptr;
    std::unordered_map<std::string, std::unique_ptr<VolumeProcessor>> m_sessions;
    std::mutex m_sessionMutex;
    int m_nextSessionId = 0;

    // 网格会话：ExecuteMeshGeneration 后保留，SaveMeshPpcf 可复用，避免重复 init + openDocument
    std::unique_ptr<GeometryAPI> m_meshGeometryAPI;
    std::string m_meshOpenDocumentPath;
    std::string m_meshSessionId;           // 几何 session 用于网格划分时，SaveMeshPpcf 可复用
    std::string m_meshSessionProjectPath;  // 与 m_meshSessionId 配套的项目 ppcf 路径
    std::mutex m_meshSessionMutex;

    std::string createSession(std::unique_ptr<VolumeProcessor> processor);
    VolumeProcessor* getSession(const std::string& sessionId);
    void removeSession(const std::string& sessionId);

    json handleImportGeometryModel(const json& params);
    json handleExecuteGeometryProcessing(const json& params);
    json handleExecuteGeometryMatching(const json& params);
    json handleExecuteMeshGeneration(const json& params);
    json handleShowGeometry(const json& params);
    json handleShowMesh(const json& params);
    json handleCloseSession(const json& params);
    json handleDeleteVolumeByName(const json& params);
    json handleGetUnmatchedVolumeNames(const json& params);
    json handleSaveGeometryPpcf(const json& params);
    json handleSaveMeshPpcf(const json& params);
    json handleGetMeshQuality(const json& params);
};
