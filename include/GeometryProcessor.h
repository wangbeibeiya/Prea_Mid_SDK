#ifndef GEOMETRYPROCESSOR_H
#define GEOMETRYPROCESSOR_H

#include "../src/GeometryAPI.h"
#include "GeometryDataStructures.h"
#include "Logger.h"
#include "ProjectModelData.h"
#include <base/pfGroupData.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <functional>

/**
 * @brief 几何处理器类
 * 
 * 负责几何识别、匹配和重命名等操作
 */
class GeometryProcessor {
public:
    /**
     * @brief 构造函数
     * @param geometryAPI 几何API实例
     * @param logger 日志管理器实例
     */
    GeometryProcessor(GeometryAPI* geometryAPI, Logger* logger);
    
    /**
     * @brief 析构函数
     */
    ~GeometryProcessor();
    
    /**
     * @brief 分析体对象（识别、匹配和重命名）
     * @param modelData 可选的 ProjectModelData 对象，用于匹配和重命名体（非const，因为需要保存重命名映射）
     * @param tolerance 包围盒匹配容差，默认 1e-6
     * @param geometryModel 输出的几何模型数据
     * @param volumeFaceGroupsMap 输出的体名称和对应的面组名称映射
     * @return 是否成功处理
     */
    bool analyzeVolumes(ProjectModelData* modelData = nullptr, 
                       double tolerance = 1e-6,
                       GeometryModel* geometryModel = nullptr,
                       std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap = nullptr);
    
    /**
     * @brief 设置进度回调函数
     * @param callback 回调函数
     */
    void setProgressCallback(std::function<void(const std::string&)> callback);

private:
    GeometryAPI* m_geometryAPI;              ///< 几何API实例（不拥有所有权）
    Logger* m_logger;                         ///< 日志管理器实例（不拥有所有权）
    std::function<void(const std::string&)> m_progressCallback; ///< 进度回调函数
    
    /**
     * @brief 输出进度信息
     */
    void printProgress(const std::string& message);
    
    /**
     * @brief 设置错误信息
     */
    void setError(const std::string& error);
    
    /**
     * @brief 计算包围盒和重心
     */
    void calculateBoundingBoxAndCentroid(const std::vector<std::vector<double>>& vertices, int volumeIndex, GeometryModel* geometryModel);
    
    /**
     * @brief 计算面的包围盒和重心
     */
    void calculateFaceBoundingBoxAndCentroid(const std::vector<std::vector<double>>& vertices, int faceIndex, const std::string& faceName, GeometryModel* geometryModel);
    
    /**
     * @brief 分析group中的真实面
     */
    void analyzeRealFacesInGroup(class PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                 int groupIndex, 
                                 const std::string& groupName, 
                                 const ProjectModelData* modelData, 
                                 double tolerance,
                                 GeometryModel* geometryModel,
                                 std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet,
                                 std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap);
    
    /**
     * @brief 分析group中的真实面的内部实现
     */
    void analyzeRealFacesInGroupImpl(class PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                     class PREPRO_BASE_NAMESPACE::PFVolume* volume, 
                                     int groupIndex, 
                                     const std::string& groupName, 
                                     const ProjectModelData* modelData, 
                                     double tolerance, 
                                     void* statsPtr, 
                                     std::vector<std::string>* faceGroupNames,
                                     GeometryModel* geometryModel,
                                     std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet,
                                     std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap);
    
    /**
     * @brief 在group中创建面组（按ID分组）
     */
    void createFaceGroupsInGroup(class PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                 const std::string& groupName, 
                                 const ProjectModelData* modelData, 
                                 double tolerance,
                                 std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet);
    
    /**
     * @brief 比较两个包围盒是否匹配
     */
    bool matchBoundingBox(const BoundingBox3D& bbox1, const BoundingBox3D& bbox2, double tolerance);
    
    /**
     * @brief 从element集合中收集唯一的顶点索引
     */
    std::set<unsigned int> collectUniqueVertexIndices(const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements);
    
    /**
     * @brief 从group获取顶点坐标
     */
    bool getVerticesFromGroup(const std::set<unsigned int>& vertexIndices, 
                             class PREPRO_BASE_NAMESPACE::PFGroup* group, 
                             std::vector<std::vector<double>>& outputVertices);
    
    /**
     * @brief 从group获取顶点坐标，返回Point3D格式
     */
    bool getVerticesFromGroup(const std::set<unsigned int>& vertexIndices, 
                             class PREPRO_BASE_NAMESPACE::PFGroup* group, 
                             std::vector<Point3D>& outputVertices);
    
    /**
     * @brief 按element ID分组
     */
    std::unordered_map<unsigned int, std::vector<class PREPRO_BASE_NAMESPACE::PFElement*>> groupElementsById(
        class PREPRO_BASE_NAMESPACE::PFElement** elements, size_t elementSize);
    
    /**
     * @brief 创建几何面数据结构对象
     */
    std::shared_ptr<GeometryFace> createGeometryFace(
        const std::vector<class PREPRO_BASE_NAMESPACE::PFElement*>& faceElements, 
        int faceId, 
        class PREPRO_BASE_NAMESPACE::PFGroup* group, 
        size_t elementCount);
    
    /**
     * @brief 创建几何体数据结构对象
     */
    std::shared_ptr<GeometryVolume> createGeometryVolume(
        class PREPRO_BASE_NAMESPACE::PFVolume* volume, 
        int volumeIndex);
    
    /**
     * @brief 计算真实面的属性
     */
    void calculateRealFaceProperties(
        const std::vector<class PREPRO_BASE_NAMESPACE::PFElement*>& faceElements, 
        int faceId, 
        class PREPRO_BASE_NAMESPACE::PFGroup* group);
    
    /**
     * @brief 分析面组并获取面组名称列表
     */
    std::vector<std::string> analyzeFacesInGroupAndGetNames(
        class PREPRO_BASE_NAMESPACE::PFGroup* group, 
        int groupIndex, 
        const std::string& groupName, 
        const ProjectModelData* modelData, 
        double tolerance, 
        void* statsPtr);
    
    /**
     * @brief 计算体的重心
     */
    void calculateVolumeCentroid(
        class PREPRO_BASE_NAMESPACE::PFVolume* volume, 
        int volumeIndex);
    
    std::string m_lastError;  ///< 最后的错误信息
};

#endif // GEOMETRYPROCESSOR_H

