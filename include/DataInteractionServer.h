#pragma once

#include "SocketCommandAPI.h"
#include "json.hpp"
using json = nlohmann::json;

class ModelProcessingServer;

/**
 * @brief 数据交互服务类
 *
 * 通过 Socket 接收命令，从会话顶层数据查询体列表、面组等。
 *
 * 体名与面组查询：始终与几何层一致。使用 ppcfPath 且复用已打开的文档时，若文档内同时有几何与网格，
 * 优先从 PFGeometry::getAllData 取体列表（几何匹配 renameVolume 后仍正确）；网格层 getAllData 可能保留
 * 划分网格时的旧体名，直至重新分网。
 *
 * RefreshSessionData：再次调用 getAllData 刷新会话内缓存；ExecuteGeometryMatching 结束后主流程已刷新，
 * 若仍遇到异常可手动调用。
 *
 * 命令（sessionId 优先，无会话时可用 ppcfPath 从文件加载）：
 * - GetVolumeListNames: 获取体列表名称
 * - GetFaceGroupNamesByVolume: 获取某个体下的面组名称
 * - RefreshSessionData: 刷新会话顶层数据
 */
class DataInteractionServer
{
public:
    /**
     * @brief 构造函数
     * @param sharedAPI 共享的 Socket 命令 API
     * @param modelServer ModelProcessingServer 指针，用于从会话获取顶层数据
     */
    explicit DataInteractionServer(SocketCommandAPI* sharedAPI, ModelProcessingServer* modelServer = nullptr);
    ~DataInteractionServer();

private:
    SocketCommandAPI* m_sharedAPI = nullptr;
    ModelProcessingServer* m_modelServer = nullptr;

    json handleGetVolumeListNames(const json& params);
    json handleGetFaceGroupNamesByVolume(const json& params);
    json handleRefreshSessionData(const json& params);
};
