#ifndef VOLUMEPROCESSOR_H
#define VOLUMEPROCESSOR_H

#include "../src/GeometryAPI.h"
#include "GeometryDataStructures.h"
#include "Logger.h"
#include "GeometryProcessor.h"
#include "MeshVisualizationServer.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <base/pfGroupData.h>

/**
 * @brief 体处理器类（仅几何识别）
 *
 * 负责几何导入、快速修复、查找体、匹配重命名等。
 * 不包含网格划分，网格划分由 MeshProcessor 独立完成。
 */
class VolumeProcessor {
public:
    /**
     * @brief 处理选项结构体
     */
    struct ProcessOptions {
        bool enableQuickRepair = true;      ///< 是否启用快速修复
        bool enableFindVolumes = true;      ///< 是否启用体积查找
        bool enableRendering = true;       ///< 是否启用渲染显示（默认不渲染）
        double repairTolerance = 1e-3;      ///< 修复容差
        
        ProcessOptions() = default;
    };

    /**
     * @brief 构造函数
     * @param examplePath 示例文件路径
     * @param appLogger 应用级日志（可选，用于 Socket 流程时写入统一日志）
     */
    explicit VolumeProcessor(const std::string& examplePath = "", class Logger* appLogger = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~VolumeProcessor();
    
    /**
     * @brief 初始化处理器
     * @param enableSocketServer 是否启动Socket服务器（默认true）
     * @return 是否成功初始化
     */
    bool initialize(bool enableSocketServer = true);
    
    /**
     * @brief 1. 导入几何模型（仅导入，顶层数据保存在处理器内供后续步骤使用）
     * @param filePath 模型文件路径
     * @param importMode 导入模式：1-首次导入，2-替换模式，3-追加模式
     * @param modelData 可选的 ProjectModelData，用于日志路径、保存路径
     * @return 是否成功
     */
    bool importGeometryModel(const std::string& filePath,
                            int importMode = 1,
                            class ProjectModelData* modelData = nullptr);

    /**
     * @brief 2. 几何处理（在导入成功的顶层数据上执行 quickRepair + findVolumes）
     * @param options 处理选项
     * @return 是否成功
     */
    bool executeGeometryProcessing(const ProcessOptions& options);

    /**
     * @brief 3. 几何识别匹配（在已有顶层数据上执行 analyzeVolumes 匹配重命名）
     * @param modelData ProjectModelData 用于 SetList 匹配
     * @return 是否成功
     */
    bool executeGeometryMatching(class ProjectModelData* modelData);

    /**
     * @brief 刷新顶层数据（几何识别匹配创建新 group 后需调用，以更新体/面组数据）
     * @return 是否成功
     */
    bool refreshGeometryData();

    /**
     * @brief 保存几何 ppcf 到 modelData 指定的路径
     * @param modelData 用于获取 WorkingDirectory、ProjectName
     * @return 是否成功
     */
    bool saveGeometryPpcf(class ProjectModelData* modelData) const;

    /**
     * @brief 保存几何 ppcf 到指定路径
     * @param ppcfPath 完整的 ppcf 文件路径
     * @return 是否成功
     */
    bool saveGeometryPpcfToPath(const std::string& ppcfPath);

    /**
     * @brief 打开 ppcf 并执行几何识别匹配，填充 modelData 的 VolumeRenameMap
     * @param ppcfPath ppcf 文件路径
     * @param modelData 已从 JSON 加载的 ProjectModelData，匹配结果写入其 VolumeRenameMap
     * @return 是否成功
     */
    bool openPpcfAndRunMatching(const std::string& ppcfPath, class ProjectModelData* modelData);

    /**
     * @brief 处理几何模型（完整流程：导入 + 几何处理 + 几何识别匹配 + 保存）
     * @param filePath 模型文件路径
     * @param importMode 导入模式：1-首次导入，2-替换模式，3-追加模式
     * @param options 处理选项
     * @param modelData 可选的 ProjectModelData 对象，用于匹配和重命名体
     * @return 是否成功处理
     */
    bool processGeometryModel(const std::string& filePath,
                              int importMode = 1,
                              const ProcessOptions& options = ProcessOptions(),
                              class ProjectModelData* modelData = nullptr);

    /**
     * @brief 从 JSON 路径执行几何匹配（导入几何、快速修复、查找体、保存几何 ppcf）
     * @param jsonPath JSON 项目文件路径，需包含 WorkingDirectory、ProjectName 等
     * @return 是否成功
     */
    bool processGeometryMatchingFromJson(const std::string& jsonPath);
    
    /**
     * @brief 处理后处理结果（导入+显示提示）
     * @param filePath 结果文件路径
     * @return 是否成功处理
     */
    bool processPostProcessResult(const std::string& filePath);
    
    /**
     * @brief 批量处理几何模型
     * @param filePaths 文件路径列表
     * @param importMode 导入模式
     * @param options 处理选项
     * @return 成功处理的文件数量
     */
    int batchProcessGeometry(const std::vector<std::string>& filePaths,
                             int importMode = 1,
                             const ProcessOptions& options = ProcessOptions());
    
    /**
     * @brief 设置默认处理选项
     * @param options 处理选项
     */
    void setDefaultProcessOptions(const ProcessOptions& options);
    
    /**
     * @brief 获取当前处理选项
     * @return 当前处理选项
     */
    ProcessOptions getCurrentProcessOptions() const;
    
    /**
     * @brief 获取支持的文件格式
     * @return 支持的文件格式列表
     */
    std::vector<std::string> getSupportedFormats() const;
    
    /**
     * @brief 显示当前模型
     * @param showGroups 是否显示组信息
     * @return 是否成功显示
     */
    bool showCurrentModel(bool showGroups = false);
    
    /**
     * @brief 获取几何API实例
     * @return GeometryAPI指针
     */
    GeometryAPI* getGeometryAPI();
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 获取处理统计信息
     * @return 统计信息字符串
     */
    std::string getProcessingStats() const;
    
    /**
     * @brief 重置处理器状态
     */
    void reset();

    /**
     * @brief 获取最近一次几何匹配中未匹配成功的体名称列表
     * @return 未匹配体名称列表，需在 ExecuteGeometryMatching 之后调用
     */
    const std::vector<std::string>& getLastUnmatchedVolumeNames() const { return m_lastUnmatchedVolumeNames; }

private:
    std::unique_ptr<GeometryAPI> m_geometryAPI;   ///< 几何API实例
    ProcessOptions m_defaultOptions;              ///< 默认处理选项
    std::string m_lastError;                      ///< 最后的错误信息
    std::string m_currentImportFilePath;          ///< 当前导入的文件路径
    
    // 日志管理器
    Logger* m_appLogger = nullptr;                ///< 应用级日志（可选，Socket 流程时使用）
    Logger m_internalLogger;                      ///< 内部日志（无 appLogger 时使用）
    
    // 几何处理器
    std::unique_ptr<GeometryProcessor> m_geometryProcessor;  ///< 几何处理器实例

    // 网格可视化服务器（Socket控制，用于几何渲染）
    std::unique_ptr<MeshVisualizationServer> m_visualizationServer;  ///< 可视化服务器实例

    // 输出控制
    bool m_outputGroupJson = false;               ///< 是否输出group.json文件（默认false）
    
    std::vector<std::string> m_lastUnmatchedVolumeNames;  ///< 最近一次几何匹配中未匹配成功的体名称

    // 统计信息
    int m_processedGeometryCount;                 ///< 已处理的几何文件数
    int m_processedMeshCount;                     ///< 已处理的网格文件数
    int m_processedPostProcessCount;              ///< 已处理的后处理文件数
    int m_successfulRepairCount;                  ///< 成功修复的次数
    int m_foundVolumeCount;                       ///< 找到的体积总数
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const std::string& error);
    
    
    /**
     * @brief 执行渲染显示
     * @param visualizationType 可视化类型
     * @return 是否成功
     */
    bool executeRendering(const std::string& visualizationType);
    
    /**
     * @brief 更新统计信息
     * @param type 处理类型："geometry", "mesh", "postprocess"
     * @param success 是否成功
     * @param volumeCount 体积数量（仅用于几何处理）
     */
    void updateStats(const std::string& type, bool success, int volumeCount = 0);
    
    /**
     * @brief 打印处理进度
     * @param message 进度信息
     */
    void printProgress(const std::string& message) const;
    
    /**
     * @brief 验证文件路径
     * @param filePath 文件路径
     * @return 是否为有效路径
     */
    bool validateFilePath(const std::string& filePath) const;

public:
    /**
     * @brief 获取分析得到的几何模型
     * @return 几何模型对象
     */
    const GeometryModel& getGeometryModel() const { return m_geometryModel; }
    GeometryModel& getGeometryModel() { return m_geometryModel; }
    
    /**
     * @brief 清理几何模型数据
     */
    void clearGeometryModel() { m_geometryModel.clear(); }

private:
    // 几何模型数据
    GeometryModel m_geometryModel;

};

#endif // VOLUMEPROCESSOR_H
