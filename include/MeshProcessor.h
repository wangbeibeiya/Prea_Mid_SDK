#ifndef MESHPROCESSOR_H
#define MESHPROCESSOR_H

#include <base/pfDocument.h>
#include <mesh/pfMesh.h>
#include <mesh/pfMeshData.h>
#include "ProjectModelData.h"
#include <string>
#include <functional>
#include <set>
#include <optional>
#include <vector>

/**
 * @brief 网格处理器类
 * 
 * 负责网格划分相关操作
 */
class MeshProcessor {
public:
    /**
     * @brief 边界层参数结构体
     */
    struct BoundaryLayerParameters {
        double firstLayerHeight = 0.001;  ///< 第一层高度
        double growthRate = 1.2;          ///< 增长率
        int layersNumber = 5;             ///< 层数
        bool isBoundaryLayers = true;    ///< 是否启用边界层
        
        BoundaryLayerParameters() = default;
    };
    
    /**
     * @brief 网格参数结构体
     */
    struct MeshParameters {
        double minSize = 0.01;              ///< 最小网格尺寸
        double maxSize = 0.1;               ///< 最大网格尺寸
        double growthRate = 1.2;             ///< 增长率
        double curvatureNormalAngle = 30.0;  ///< 曲率法向角度
        bool proximityEnabled = true;        ///< 是否启用邻近检测
        int cellsPerGap = 3;                 ///< 间隙单元数
        double minimumGapSize = 0.01;       ///< 最小间隙尺寸
        bool allTrianglesEnabled = true;     ///< 是否全部三角形
        bool meshQualityOptimizationEnabled = true; ///< 是否启用网格质量优化
        double inflationMinimumQuality = 0.05; ///< 膨胀最小质量
        double inflationSeparatingAngle = 60.0; ///< 膨胀分离角度
        double inflationMaximumHeightBaseRatio = 1.3; ///< 膨胀最大高度基比
        
        // 边界层参数（可选）
        std::optional<BoundaryLayerParameters> boundaryLayerParams;  ///< 边界层参数（如果设置，将在全局参数设置后自动应用）
        std::optional<std::string> fluidZoneSetName;                ///< 流体区域集合名称（用于边界层设置）
        std::set<std::string> excludedBoundaryNames;                ///< 排除的边界名称列表（非Wall类型的边界条件，如VelocityInlet、PressureOutlet等）
        
        MeshParameters() = default;
    };
    
    /**
     * @brief 构造函数
     * @param pfDocument PFDocument指针（不拥有所有权）
     */
    explicit MeshProcessor(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument);
    
    /**
     * @brief 析构函数
     */
    ~MeshProcessor();
    
    /**
     * @brief 初始化网格处理器
     * @return 是否成功初始化
     */
    bool initialize();
    
    /**
     * @brief 设置网格全局参数
     * @param parameters 网格参数
     * @param modelData 项目模型数据（用于边界层设置，可选）
     * @return 是否成功设置
     * @note 如果parameters中包含边界层参数，将在设置完全局参数后自动设置边界层
     */
    bool setGlobalParameters(const MeshParameters& parameters, const ProjectModelData* modelData = nullptr);
    
    /**
     * @brief 生成表面网格
     * @return 是否成功生成
     */
    bool createSurfaceMesh();
    
    /**
     * @brief 根据几何生成体积网格
     * @param withInflation 是否使用膨胀层
     * @param withPolyhedron 是否使用多面体
     * @return 是否成功生成
     */
    bool createVolumeMeshByGeometry(bool withInflation = false, bool withPolyhedron = false);
    
    /**
     * @brief 根据表面网格生成体积网格
     * @param withInflation 是否使用膨胀层
     * @param withPolyhedron 是否使用多面体
     * @return 是否成功生成
     */
    bool createVolumeMeshBySurfaceMesh(bool withInflation = false, bool withPolyhedron = false);
    
    /**
     * @brief 根据FluidZoneSet设置边界层参数
     * @param fluidZoneSetName 流体区域集合名称（对应SetList中的SetName）
     * @param parameters 边界层参数
     * @param modelData 项目模型数据（包含SetList信息，可选）
     * @param excludedBoundaryNames 需要排除的边界名称列表（非Wall类型的边界，如VelocityInlet、PressureOutlet等对应的面组名称）
     * @return 是否成功设置
     */
    bool setBoundaryLayersByFluidZoneSet(const std::string& fluidZoneSetName, 
                                         const BoundaryLayerParameters& parameters,
                                         const ProjectModelData* modelData = nullptr,
                                         const std::set<std::string>& excludedBoundaryNames = std::set<std::string>());
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    std::string getLastError() const { return m_lastError; }
    
    /**
     * @brief 验证网格数据是否存在
     * @return 如果存在网格数据返回true，否则返回false
     */
    bool hasMeshData() const;

    /**
     * @brief 获取网格环境中的数据（仅网格，不包含几何）
     * @param data 输出PFData
     * @return 是否成功获取
     */
    bool getMeshData(PREPRO_BASE_NAMESPACE::PFData& data) const;
    
    /**
     * @brief 设置进度回调函数
     * @param callback 回调函数
     */
    void setProgressCallback(std::function<void(const std::string&)> callback);

private:
    PREPRO_BASE_NAMESPACE::PFDocument* m_pfDocument;  ///< PFDocument指针（不拥有所有权）
    class PREPRO_MESH_NAMESPACE::PFMesh* m_pfMesh;     ///< PFMesh指针（不拥有所有权）
    std::string m_lastError;                            ///< 最后的错误信息
    std::function<void(const std::string&)> m_progressCallback; ///< 进度回调函数
    
    /**
     * @brief 输出进度信息
     */
    void printProgress(const std::string& message);
    
    /**
     * @brief 设置错误信息
     */
    void setError(const std::string& error);
};

#endif // MESHPROCESSOR_H

