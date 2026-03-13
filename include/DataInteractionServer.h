#pragma once

#include "SocketCommandAPI.h"
#include "json.hpp"
using json = nlohmann::json;

class ModelProcessingServer;

/**
 * @brief 数据交互服务类
 *
 * 通过 Socket 接收命令，从会话顶层数据查询体列表、面组等。
 * 几何识别匹配创建新 group 后需先调用 RefreshSessionData 刷新顶层数据。
 *
 * 命令（sessionId 优先，无会话时可用 ppcfPath 从文件加载）：
 * - GetVolumeListNames: 获取体列表名称
 * - GetFaceGroupNamesByVolume: 获取某个体下的面组名称
 * - RefreshSessionData: 刷新会话顶层数据（匹配后创建新 group 时必调）
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
