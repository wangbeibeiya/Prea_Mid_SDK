#ifndef MODELPROCESSOR_H
#define MODELPROCESSOR_H

#include "../src/GeometryAPI.h"
#include "GeometryDataStructures.h"
#include "Logger.h"
#include "GeometryProcessor.h"
#include "MeshProcessor.h"
#include "MeshVisualizationServer.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <base/pfGroupData.h>

/**
 * @brief 模型处理器类
 * 
 * 负责模型的导入、映射、处理和渲染流程管理
 * 包括自动执行quickRepair、findVolumes等操作
 */
class ModelProcessor {
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
     */
    explicit ModelProcessor(const std::string& examplePath = "");
    
    /**
     * @brief 析构函数
     */
    ~ModelProcessor();
    
    /**
     * @brief 初始化处理器
     * @param enableSocketServer 是否启动Socket服务器（默认true）
     * @return 是否成功初始化
     */
    bool initialize(bool enableSocketServer = true);
    
    /**
     * @brief 处理几何模型（导入+处理+渲染）
     * @param filePath 模型文件路径
     * @param importMode 导入模式：1-首次导入，2-替换模式，3-追加模式
     * @param options 处理选项
     * @param modelData 可选的 ProjectModelData 对象，用于匹配和重命名体（非const，因为需要保存重命名映射）
     * @return 是否成功处理
     */
    bool processGeometryModel(const std::string& filePath, 
                              int importMode = 1, 
                              const ProcessOptions& options = ProcessOptions(),
                              class ProjectModelData* modelData = nullptr);
    
    /**
     * @brief 处理网格模型（导入+渲染）
     * @param filePath 网格文件路径
     * @param enableRendering 是否启用渲染
     * @return 是否成功处理
     */
    bool processMeshModel(const std::string& filePath, bool enableRendering = true);
    
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

private:
    std::unique_ptr<GeometryAPI> m_geometryAPI;   ///< 几何API实例
    ProcessOptions m_defaultOptions;              ///< 默认处理选项
    std::string m_lastError;                      ///< 最后的错误信息
    std::string m_currentImportFilePath;          ///< 当前导入的文件路径
    
    // 日志管理器
    Logger m_logger;                              ///< 日志管理器实例
    
    // 几何处理器和网格处理器
    std::unique_ptr<GeometryProcessor> m_geometryProcessor;  ///< 几何处理器实例
    std::unique_ptr<MeshProcessor> m_meshProcessor;            ///< 网格处理器实例
    
    // 网格可视化服务器（Socket控制）
    std::unique_ptr<MeshVisualizationServer> m_visualizationServer;  ///< 可视化服务器实例
    
    // 输出控制
    bool m_outputGroupJson = false;               ///< 是否输出group.json文件（默认false）
    bool m_enableMeshGeneration = true;          ///< 是否在保存ppcf之前生成网格（默认false）
    
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
     * @brief 执行几何处理流程
     * @param options 处理选项
     * @param modelData 可选的 ProjectModelData 对象，用于匹配和重命名体（非const，因为需要保存重命名映射）
     * @return 是否成功
     */
    bool executeGeometryProcessing(const ProcessOptions& options, class ProjectModelData* modelData = nullptr);
    
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

#endif // MODELPROCESSOR_H
