#pragma once

#include "GeometryDataStructures.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <utility>
#include <set>

// JSON 库（使用 nlohmann/json）
// 使用单头文件版本，直接包含头文件
#include <json.hpp>

/**
 * @brief 模型项类 - 用于 ModelTree
 */
class ModelItem
{
public:
    ModelItem() : Id(0), Index(0), Volume(0.0) {}
    
    int Id;                         ///< 对象在CAD中的ID
    int Index;                      ///< 在ModelTree中的索引
    std::string Path;               ///< 对象在CAD中的路径
    Point3D BaryCenter;             ///< 重心
    double Volume;                  ///< 体积
    BoundingBox3D BoundingBox;      ///< 包围盒
};

/**
 * @brief 集合项基类
 */
class SetItemBase
{
public:
    SetItemBase() = default;
    virtual ~SetItemBase() = default;
    
    std::optional<std::string> SetName;     ///< 集合名称
    std::optional<std::string> SetType;    ///< 集合类型：Solid/Face/Edge/Point
    
    // JSON序列化
    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& json) = 0;
    virtual std::shared_ptr<SetItemBase> clone() const = 0;
};

/**
 * @brief 实体项类 - 用于 SetList 和 AllSolidList
 */
class SolidItem
{
public:
    SolidItem() : Index(0), Id(0), Volume(0.0) {}
    
    int Index;                      ///< 在ModelTree中的索引
    int Id;                         ///< 对象在CAD中的ID
    std::string Path;               ///< 对象在CAD中的路径
    Point3D BaryCenter;             ///< 重心
    double Volume;                  ///< 体积
    BoundingBox3D BoundingBox;      ///< 包围盒
    
    // JSON序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
};

/**
 * @brief 面项类 - 用于 SetList
 */
class FaceItem
{
public:
    FaceItem() : Index(0), Id(0), Area(0.0) {}
    
    int Index;                      ///< 在ModelTree中的索引
    int Id;                         ///< 对象在CAD中的ID
    std::string Path;               ///< 对象在CAD中的路径
    Point3D Centroid;               ///< 形心
    Point3D FacePoint;              ///< 面上一点
    std::vector<Point3D> Vertices;  ///< 围成面的顶点
    BoundingBox3D BoundingBox;      ///< 包围盒
    double Area;                    ///< 面积
    
    // JSON序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
};

/**
 * @brief 实体集合类
 */
class SetSolidItem : public SetItemBase
{
public:
    SetSolidItem() {
        SetType = "Solid";
    }
    
    std::vector<SolidItem> Items;  ///< 集合成员
    
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
    std::shared_ptr<SetItemBase> clone() const override;
};

/**
 * @brief 面集合类
 */
class SetFaceItem : public SetItemBase
{
public:
    SetFaceItem() {
        SetType = "Face";
    }
    
    std::vector<FaceItem> Items;   ///< 集合成员
    
    nlohmann::json toJson() const override;
    void fromJson(const nlohmann::json& json) override;
    std::shared_ptr<SetItemBase> clone() const override;
};

/**
 * @brief 流体网格信息结构体
 */
struct FluidMeshInfo
{
    std::optional<double> GrowthRate;      ///< 增长率
    std::optional<double> MaxMeshSize;     ///< 最大网格尺寸
    std::optional<double> MinMeshSize;     ///< 最小网格尺寸
    std::optional<double> NormalAngle;     ///< 法向角度
    
    FluidMeshInfo() = default;
    
    // JSON序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
};

/**
 * @brief 局部流体网格信息项
 * 对应 LocalFluidMeshsInfo.LocalFluidMeshsInfo 数组中的单个元素
 */
struct LocalFluidMeshItem
{
    std::optional<std::string> Name;           ///< 局部网格名称
    std::optional<std::string> RefinementSet;   ///< 参考集合名称（对应 SetList 中的 SetName）
    std::optional<double> MinMeshSize;          ///< 最小网格尺寸（单位：毫米，与 FluidMeshInfo 一致）
    std::optional<double> MaxMeshSize;          ///< 最大网格尺寸（单位：毫米）
    std::optional<double> GrowthRate;          ///< 增长率
    std::optional<double> NormalAngle;         ///< 曲率法向角度
    
    LocalFluidMeshItem() = default;
    
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
};

/**
 * @brief 项目模型数据类
 * 包含项目配置数据（工作目录、项目名称等）和模型集合数据（模型树、集合列表等）
 */
class ProjectModelData
{
public:
    ProjectModelData();
    ~ProjectModelData();
    
    // 拷贝构造函数和赋值操作符
    ProjectModelData(const ProjectModelData& other);
    ProjectModelData& operator=(const ProjectModelData& other);
    
    // JSON序列化
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
    bool loadFromJsonFile(const std::string& filePath);
    bool saveToJsonFile(const std::string& filePath) const;
    void reset();
    
    // Get/Set 方法
    std::string getName() const { return Name.value_or(""); }
    void setName(const std::string& value) { Name = value; }
    
    std::string getDescription() const { return Description.value_or(""); }
    void setDescription(const std::string& value) { Description = value; }
    
    bool getIsEnabled() const { return IsEnabled.value_or(false); }
    void setIsEnabled(bool value) { IsEnabled = value; }
    
    std::string getWorkingDirectory() const { return WorkingDirectory.value_or(""); }
    void setWorkingDirectory(const std::string& value) { WorkingDirectory = value; }
    
    std::string getProjectName() const { return ProjectName.value_or(""); }
    void setProjectName(const std::string& value) { ProjectName = value; }
    
    std::string getGeometryPath() const { return GeometryPath.value_or(""); }
    void setGeometryPath(const std::string& value) { GeometryPath = value; }
    
    std::string getOriginalGeometryPath() const { return OriginalGeometryPath.value_or(""); }
    void setOriginalGeometryPath(const std::string& value) { OriginalGeometryPath = value; }
    
    // FluidMeshInfo 访问方法
    const FluidMeshInfo& getFluidMeshInfo() const { return FluidMeshInfo; }
    FluidMeshInfo& getFluidMeshInfo() { return FluidMeshInfo; }
    void setFluidMeshInfo(const FluidMeshInfo& info) { FluidMeshInfo = info; }
    
    // LocalFluidMeshsInfo 访问方法
    const std::vector<LocalFluidMeshItem>& getLocalFluidMeshsInfo() const { return LocalFluidMeshsInfo; }
    std::vector<LocalFluidMeshItem>& getLocalFluidMeshsInfo() { return LocalFluidMeshsInfo; }
    
    // === ModelTree 基本访问 ===
    const std::vector<std::shared_ptr<ModelItem>>& GetModelTree() const { return ModelTree; }
    std::vector<std::shared_ptr<ModelItem>>& GetModelTree() { return ModelTree; }
    void SetModelTree(const std::vector<std::shared_ptr<ModelItem>>& tree) { ModelTree = tree; }
    void AddModelItem(std::shared_ptr<ModelItem> item);
    
    // === SetList 基本访问 ===
    const std::vector<std::shared_ptr<SetItemBase>>& GetSetList() const { return SetList; }
    std::vector<std::shared_ptr<SetItemBase>>& GetSetList() { return SetList; }
    void SetSetList(const std::vector<std::shared_ptr<SetItemBase>>& list) { SetList = list; }
    void AddSetItem(std::shared_ptr<SetItemBase> set);
    
    // === AllSolidList 基本访问 ===
    const std::vector<SolidItem>& GetAllSolidList() const { return AllSolidList; }
    std::vector<SolidItem>& GetAllSolidList() { return AllSolidList; }
    void SetAllSolidList(const std::vector<SolidItem>& list) { AllSolidList = list; }
    void AddSolidItem(const SolidItem& item);
    void ClearAllSolidList() { AllSolidList.clear(); }
    
    // 统计信息
    size_t getModelTreeCount() const { return ModelTree.size(); }
    size_t getSetListCount() const { return SetList.size(); }
    size_t getAllSolidListCount() const { return AllSolidList.size(); }
    
    // 打印信息
    void printInfo() const;
    void printSummary() const;

    /**
     * @brief 记录体对象重命名映射（原名称 -> 新名称）
     * 仅用于运行时几何名称匹配，不参与JSON序列化
     */
    void addVolumeRename(const std::string& oldName, const std::string& newName);

    /**
     * @brief 根据新名称查找原始名称
     * @param newName 新名称（例如重命名后的体名称或SetName）
     * @return 如果存在映射则返回原始名称，否则返回传入的newName
     */
    std::string mapNewVolumeNameToOld(const std::string& newName) const;

    /**
     * @brief 获取所有非Wall类型的边界条件名称集合
     * @return 非Wall类型边界条件的Name集合（例如 "INLET", "OUTLET" 等）
     * @note 这些边界条件对应的面不需要设置边界层
     */
    std::set<std::string> getNonWallBoundaryConditionNames() const;

private:
    std::optional<std::string> Name;
    std::optional<std::string> Description;
    std::optional<bool> IsEnabled;
    std::optional<std::string> WorkingDirectory;
    std::optional<std::string> ProjectName;
    std::optional<std::string> GeometryPath;
    std::optional<std::string> OriginalGeometryPath;
    
    // 流体网格信息
    FluidMeshInfo FluidMeshInfo;
    
    // 局部流体网格信息列表
    std::vector<LocalFluidMeshItem> LocalFluidMeshsInfo;
    
    // 模型树
    std::vector<std::shared_ptr<ModelItem>> ModelTree;
    
    // 集合列表
    std::vector<std::shared_ptr<SetItemBase>> SetList;
    
    // 所有实体列表
    std::vector<SolidItem> AllSolidList;

    // 体对象重命名映射：first为原名称，second为新名称
    std::vector<std::pair<std::string, std::string>> VolumeRenameMap;
};

