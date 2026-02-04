#include "../include/ProjectModelData.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>

// 使用 nlohmann/json 库（单头文件版本）
#include <json.hpp>
using json = nlohmann::json;

// ============= FluidMeshInfo 实现 =============

json FluidMeshInfo::toJson() const
{
    json j;
    if (GrowthRate.has_value()) j["GrowthRate"] = GrowthRate.value();
    if (MaxMeshSize.has_value()) j["MaxMeshSize"] = MaxMeshSize.value();
    if (MinMeshSize.has_value()) j["MinMeshSize"] = MinMeshSize.value();
    if (NormalAngle.has_value()) j["NormalAngle"] = NormalAngle.value();
    return j;
}

void FluidMeshInfo::fromJson(const json& j)
{
    if (j.contains("GrowthRate")) GrowthRate = j["GrowthRate"].get<double>();
    if (j.contains("MaxMeshSize")) MaxMeshSize = j["MaxMeshSize"].get<double>();
    if (j.contains("MinMeshSize")) MinMeshSize = j["MinMeshSize"].get<double>();
    if (j.contains("NormalAngle")) NormalAngle = j["NormalAngle"].get<double>();
}

// ============= SolidItem 实现 =============

json SolidItem::toJson() const
{
    json j;
    j["Index"] = Index;
    j["Id"] = Id;
    j["Path"] = Path;
    j["BaryCenter"] = {{"X", BaryCenter.x}, {"Y", BaryCenter.y}, {"Z", BaryCenter.z}};
    j["Volume"] = Volume;
    j["BoundingBox"] = {
        {"Center", {{"X", BoundingBox.center.x}, {"Y", BoundingBox.center.y}, {"Z", BoundingBox.center.z}}},
        {"Size", {{"X", BoundingBox.size.x}, {"Y", BoundingBox.size.y}, {"Z", BoundingBox.size.z}}},
        {"Min", {{"X", BoundingBox.minPoint.x}, {"Y", BoundingBox.minPoint.y}, {"Z", BoundingBox.minPoint.z}}},
        {"Max", {{"X", BoundingBox.maxPoint.x}, {"Y", BoundingBox.maxPoint.y}, {"Z", BoundingBox.maxPoint.z}}}
    };
    return j;
}

void SolidItem::fromJson(const json& j)
{
    if (j.contains("Index")) Index = j["Index"].get<int>();
    if (j.contains("Id")) Id = j["Id"].get<int>();
    if (j.contains("Path")) Path = j["Path"].get<std::string>();
    if (j.contains("BaryCenter")) {
        auto bc = j["BaryCenter"];
        BaryCenter.x = bc.value("X", 0.0);
        BaryCenter.y = bc.value("Y", 0.0);
        BaryCenter.z = bc.value("Z", 0.0);
    }
    if (j.contains("Volume")) Volume = j["Volume"].get<double>();
    
    // 支持 BoundingBox、AABBBox 和 AabbBox 三种格式
    const json* bb = nullptr;
    if (j.contains("BoundingBox")) {
        bb = &j["BoundingBox"];
    } else if (j.contains("AABBBox")) {
        bb = &j["AABBBox"];
    } else if (j.contains("AabbBox")) {
        bb = &j["AabbBox"];
    }
    
    if (bb != nullptr) {
        // 处理 PointA 和 PointB（AABBBox/AabbBox 格式）
        if (bb->contains("PointA") && bb->contains("PointB")) {
            auto pointA = (*bb)["PointA"];
            auto pointB = (*bb)["PointB"];
            double ax = pointA.value("X", 0.0);
            double ay = pointA.value("Y", 0.0);
            double az = pointA.value("Z", 0.0);
            double bx = pointB.value("X", 0.0);
            double by = pointB.value("Y", 0.0);
            double bz = pointB.value("Z", 0.0);
            
            // 确定 min 和 max（PointA 和 PointB 可能不是按顺序排列的）
            BoundingBox.minPoint.x = std::min(ax, bx);
            BoundingBox.minPoint.y = std::min(ay, by);
            BoundingBox.minPoint.z = std::min(az, bz);
            BoundingBox.maxPoint.x = std::max(ax, bx);
            BoundingBox.maxPoint.y = std::max(ay, by);
            BoundingBox.maxPoint.z = std::max(az, bz);
        }
        // 处理 Min 和 Max（标准格式）
        else if (bb->contains("Min") && bb->contains("Max")) {
            auto min = (*bb)["Min"];
            BoundingBox.minPoint.x = min.value("X", 0.0);
            BoundingBox.minPoint.y = min.value("Y", 0.0);
            BoundingBox.minPoint.z = min.value("Z", 0.0);
            
            auto max = (*bb)["Max"];
            BoundingBox.maxPoint.x = max.value("X", 0.0);
            BoundingBox.maxPoint.y = max.value("Y", 0.0);
            BoundingBox.maxPoint.z = max.value("Z", 0.0);
        }
        
        // 如果提供了 Center 和 Size，直接使用
        if (bb->contains("Center")) {
            auto c = (*bb)["Center"];
            BoundingBox.center.x = c.value("X", 0.0);
            BoundingBox.center.y = c.value("Y", 0.0);
            BoundingBox.center.z = c.value("Z", 0.0);
        } else {
            // 如果没有 Center，从 Min 和 Max 计算
            BoundingBox.center.x = (BoundingBox.minPoint.x + BoundingBox.maxPoint.x) / 2.0;
            BoundingBox.center.y = (BoundingBox.minPoint.y + BoundingBox.maxPoint.y) / 2.0;
            BoundingBox.center.z = (BoundingBox.minPoint.z + BoundingBox.maxPoint.z) / 2.0;
        }
        
        if (bb->contains("Size")) {
            auto s = (*bb)["Size"];
            BoundingBox.size.x = s.value("X", 0.0);
            BoundingBox.size.y = s.value("Y", 0.0);
            BoundingBox.size.z = s.value("Z", 0.0);
        } else {
            // 如果没有 Size，从 Min 和 Max 计算
            BoundingBox.size.x = BoundingBox.maxPoint.x - BoundingBox.minPoint.x;
            BoundingBox.size.y = BoundingBox.maxPoint.y - BoundingBox.minPoint.y;
            BoundingBox.size.z = BoundingBox.maxPoint.z - BoundingBox.minPoint.z;
        }
    }
}

// ============= FaceItem 实现 =============

json FaceItem::toJson() const
{
    json j;
    j["Index"] = Index;
    j["Id"] = Id;
    j["Path"] = Path;
    j["Centroid"] = {{"X", Centroid.x}, {"Y", Centroid.y}, {"Z", Centroid.z}};
    j["FacePoint"] = {{"X", FacePoint.x}, {"Y", FacePoint.y}, {"Z", FacePoint.z}};
    j["Area"] = Area;
    
    json verticesArray = json::array();
    for (const auto& v : Vertices) {
        verticesArray.push_back({{"X", v.x}, {"Y", v.y}, {"Z", v.z}});
    }
    j["Vertices"] = verticesArray;
    
    j["BoundingBox"] = {
        {"Center", {{"X", BoundingBox.center.x}, {"Y", BoundingBox.center.y}, {"Z", BoundingBox.center.z}}},
        {"Size", {{"X", BoundingBox.size.x}, {"Y", BoundingBox.size.y}, {"Z", BoundingBox.size.z}}},
        {"Min", {{"X", BoundingBox.minPoint.x}, {"Y", BoundingBox.minPoint.y}, {"Z", BoundingBox.minPoint.z}}},
        {"Max", {{"X", BoundingBox.maxPoint.x}, {"Y", BoundingBox.maxPoint.y}, {"Z", BoundingBox.maxPoint.z}}}
    };
    
    return j;
}

void FaceItem::fromJson(const json& j)
{
    if (j.contains("Index")) Index = j["Index"].get<int>();
    if (j.contains("Id")) Id = j["Id"].get<int>();
    if (j.contains("Path")) Path = j["Path"].get<std::string>();
    if (j.contains("Centroid")) {
        auto c = j["Centroid"];
        Centroid.x = c.value("X", 0.0);
        Centroid.y = c.value("Y", 0.0);
        Centroid.z = c.value("Z", 0.0);
    }
    if (j.contains("FacePoint")) {
        auto fp = j["FacePoint"];
        FacePoint.x = fp.value("X", 0.0);
        FacePoint.y = fp.value("Y", 0.0);
        FacePoint.z = fp.value("Z", 0.0);
    }
    if (j.contains("Area")) Area = j["Area"].get<double>();
    if (j.contains("Vertices") && j["Vertices"].is_array()) {
        Vertices.clear();
        for (const auto& v : j["Vertices"]) {
            Point3D vertex;
            vertex.x = v.value("X", 0.0);
            vertex.y = v.value("Y", 0.0);
            vertex.z = v.value("Z", 0.0);
            Vertices.push_back(vertex);
        }
    }
    // 支持 BoundingBox 和 AABBBox 两种格式
    const json* bb = nullptr;
    if (j.contains("BoundingBox")) {
        bb = &j["BoundingBox"];
    } else if (j.contains("AABBBox")) {
        bb = &j["AABBBox"];
    }
    
    if (bb != nullptr) {
        // 处理 Center
        if (bb->contains("Center")) {
            auto c = (*bb)["Center"];
            BoundingBox.center.x = c.value("X", 0.0);
            BoundingBox.center.y = c.value("Y", 0.0);
            BoundingBox.center.z = c.value("Z", 0.0);
        }
        
        // 处理 Size
        if (bb->contains("Size")) {
            auto s = (*bb)["Size"];
            BoundingBox.size.x = s.value("X", 0.0);
            BoundingBox.size.y = s.value("Y", 0.0);
            BoundingBox.size.z = s.value("Z", 0.0);
        }
        
        // 处理 Min 和 Max（标准格式）
        if (bb->contains("Min")) {
            auto min = (*bb)["Min"];
            BoundingBox.minPoint.x = min.value("X", 0.0);
            BoundingBox.minPoint.y = min.value("Y", 0.0);
            BoundingBox.minPoint.z = min.value("Z", 0.0);
        }
        if (bb->contains("Max")) {
            auto max = (*bb)["Max"];
            BoundingBox.maxPoint.x = max.value("X", 0.0);
            BoundingBox.maxPoint.y = max.value("Y", 0.0);
            BoundingBox.maxPoint.z = max.value("Z", 0.0);
        }
        
        // 处理 PointA 和 PointB（AABBBox 格式）
        if (bb->contains("PointA") && bb->contains("PointB")) {
            auto pointA = (*bb)["PointA"];
            auto pointB = (*bb)["PointB"];
            
            double ax = pointA.value("X", 0.0);
            double ay = pointA.value("Y", 0.0);
            double az = pointA.value("Z", 0.0);
            double bx = pointB.value("X", 0.0);
            double by = pointB.value("Y", 0.0);
            double bz = pointB.value("Z", 0.0);
            
            // 确定 min 和 max（PointA 和 PointB 可能不是按顺序排列的）
            BoundingBox.minPoint.x = std::min(ax, bx);
            BoundingBox.minPoint.y = std::min(ay, by);
            BoundingBox.minPoint.z = std::min(az, bz);
            BoundingBox.maxPoint.x = std::max(ax, bx);
            BoundingBox.maxPoint.y = std::max(ay, by);
            BoundingBox.maxPoint.z = std::max(az, bz);
        }
        
        // 如果没有 Center，从 Min 和 Max 计算
        if (!bb->contains("Center") && bb->contains("Min") && bb->contains("Max")) {
            BoundingBox.center.x = (BoundingBox.minPoint.x + BoundingBox.maxPoint.x) / 2.0;
            BoundingBox.center.y = (BoundingBox.minPoint.y + BoundingBox.maxPoint.y) / 2.0;
            BoundingBox.center.z = (BoundingBox.minPoint.z + BoundingBox.maxPoint.z) / 2.0;
        }
        
        // 如果没有 Size，从 Min 和 Max 计算
        if (!bb->contains("Size") && bb->contains("Min") && bb->contains("Max")) {
            BoundingBox.size.x = BoundingBox.maxPoint.x - BoundingBox.minPoint.x;
            BoundingBox.size.y = BoundingBox.maxPoint.y - BoundingBox.minPoint.y;
            BoundingBox.size.z = BoundingBox.maxPoint.z - BoundingBox.minPoint.z;
        }
    }
}

// ============= SetSolidItem 实现 =============

json SetSolidItem::toJson() const
{
    json j;
    if (SetName.has_value()) j["SetName"] = SetName.value();
    if (SetType.has_value()) j["SetType"] = SetType.value();
    
    json itemsArray = json::array();
    for (const auto& item : Items) {
        itemsArray.push_back(item.toJson());
    }
    j["Items"] = itemsArray;
    
    return j;
}

void SetSolidItem::fromJson(const json& j)
{
    if (j.contains("SetName")) SetName = j["SetName"].get<std::string>();
    if (j.contains("SetType")) SetType = j["SetType"].get<std::string>();
    
    Items.clear();
    if (j.contains("Items") && j["Items"].is_array()) {
        for (const auto& itemJson : j["Items"]) {
            SolidItem item;
            item.fromJson(itemJson);
            Items.push_back(item);
        }
    }
}

std::shared_ptr<SetItemBase> SetSolidItem::clone() const
{
    auto newSet = std::make_shared<SetSolidItem>();
    newSet->SetName = this->SetName;
    newSet->SetType = this->SetType;
    newSet->Items = this->Items;
    return newSet;
}

// ============= SetFaceItem 实现 =============

json SetFaceItem::toJson() const
{
    json j;
    if (SetName.has_value()) j["SetName"] = SetName.value();
    if (SetType.has_value()) j["SetType"] = SetType.value();
    
    json itemsArray = json::array();
    for (const auto& item : Items) {
        itemsArray.push_back(item.toJson());
    }
    j["Items"] = itemsArray;
    
    return j;
}

void SetFaceItem::fromJson(const json& j)
{
    if (j.contains("SetName")) SetName = j["SetName"].get<std::string>();
    if (j.contains("SetType")) SetType = j["SetType"].get<std::string>();
    
    Items.clear();
    if (j.contains("Items") && j["Items"].is_array()) {
        for (const auto& itemJson : j["Items"]) {
            FaceItem item;
            item.fromJson(itemJson);
            Items.push_back(item);
        }
    }
}

std::shared_ptr<SetItemBase> SetFaceItem::clone() const
{
    auto newSet = std::make_shared<SetFaceItem>();
    newSet->SetName = this->SetName;
    newSet->SetType = this->SetType;
    newSet->Items = this->Items;
    return newSet;
}

// ============= ProjectModelData 实现 =============

ProjectModelData::ProjectModelData()
{
}

ProjectModelData::~ProjectModelData()
{
}

ProjectModelData::ProjectModelData(const ProjectModelData& other)
{
    Name = other.Name;
    Description = other.Description;
    IsEnabled = other.IsEnabled;
    WorkingDirectory = other.WorkingDirectory;
    ProjectName = other.ProjectName;
    GeometryPath = other.GeometryPath;
    OriginalGeometryPath = other.OriginalGeometryPath;
    ModelTree = other.ModelTree;
    SetList = other.SetList;
    AllSolidList = other.AllSolidList;
    VolumeRenameMap = other.VolumeRenameMap;
}

ProjectModelData& ProjectModelData::operator=(const ProjectModelData& other)
{
    if (this != &other) {
        Name = other.Name;
        Description = other.Description;
        IsEnabled = other.IsEnabled;
        WorkingDirectory = other.WorkingDirectory;
        ProjectName = other.ProjectName;
        GeometryPath = other.GeometryPath;
        OriginalGeometryPath = other.OriginalGeometryPath;
        ModelTree = other.ModelTree;
        SetList = other.SetList;
        AllSolidList = other.AllSolidList;
        VolumeRenameMap = other.VolumeRenameMap;
    }
    return *this;
}

json ProjectModelData::toJson() const
{
    json j;
    
    if (Name.has_value()) j["Name"] = Name.value();
    if (Description.has_value()) j["Description"] = Description.value();
    if (IsEnabled.has_value()) j["IsEnabled"] = IsEnabled.value();
    if (WorkingDirectory.has_value()) j["WorkingDirectory"] = WorkingDirectory.value();
    if (ProjectName.has_value()) j["ProjectName"] = ProjectName.value();
    if (GeometryPath.has_value()) j["GeometryPath"] = GeometryPath.value();
    if (OriginalGeometryPath.has_value()) j["OriginalGeometryPath"] = OriginalGeometryPath.value();
    
    // ModelTree
    json modelTreeArray = json::array();
    for (const auto& item : ModelTree) {
        if (!item) continue;
        json itemJson;
        itemJson["Id"] = item->Id;
        itemJson["Index"] = item->Index;
        itemJson["Path"] = item->Path;
        itemJson["BaryCenter"] = {{"X", item->BaryCenter.x}, {"Y", item->BaryCenter.y}, {"Z", item->BaryCenter.z}};
        itemJson["Volume"] = item->Volume;
        modelTreeArray.push_back(itemJson);
    }
    j["ModelTree"] = modelTreeArray;
    
    // SetList
    json setListArray = json::array();
    for (const auto& set : SetList) {
        if (!set) continue;
        setListArray.push_back(set->toJson());
    }
    j["SetList"] = setListArray;
    
    // AllSolidList
    json allSolidListArray = json::array();
    for (const auto& solidItem : AllSolidList) {
        allSolidListArray.push_back(solidItem.toJson());
    }
    j["AllSolidList"] = allSolidListArray;
    
    // FluidMeshInfo
    j["FluidMeshInfo"] = FluidMeshInfo.toJson();
    
    return j;
}

void ProjectModelData::fromJson(const json& j)
{
    // 基本属性
    try {
        if (j.contains("Name")) {
            if (j["Name"].is_null()) {
                throw json::type_error::create(302, "字段 'Name' 为null，期望string类型", &j);
            }
            Name = j["Name"].get<std::string>();
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'Name': " + std::string(e.what()), &j);
    }
    
    try {
        if (j.contains("Description")) {
            if (j["Description"].is_null()) {
                throw json::type_error::create(302, "字段 'Description' 为null，期望string类型", &j);
            }
            Description = j["Description"].get<std::string>();
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'Description': " + std::string(e.what()), &j);
    }
    
    try {
        if (j.contains("IsEnabled")) {
            if (j["IsEnabled"].is_null()) {
                throw json::type_error::create(302, "字段 'IsEnabled' 为null，期望bool类型", &j);
            }
            IsEnabled = j["IsEnabled"].get<bool>();
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'IsEnabled': " + std::string(e.what()), &j);
    }
    
    // WorkingDirectory 和 ProjectName 可能在根级别，也可能在 ProjectInfo 中
    try {
        if (j.contains("WorkingDirectory")) {
            if (j["WorkingDirectory"].is_null()) {
                throw json::type_error::create(302, "字段 'WorkingDirectory' 为null，期望string类型", &j);
            }
            WorkingDirectory = j["WorkingDirectory"].get<std::string>();
        } else if (j.contains("ProjectInfo") && j["ProjectInfo"].is_object()) {
            const auto& projectInfo = j["ProjectInfo"];
            if (projectInfo.contains("WorkingDirectory")) {
                if (projectInfo["WorkingDirectory"].is_null()) {
                    throw json::type_error::create(302, "字段 'ProjectInfo.WorkingDirectory' 为null，期望string类型", &j);
                }
                WorkingDirectory = projectInfo["WorkingDirectory"].get<std::string>();
            }
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'WorkingDirectory': " + std::string(e.what()), &j);
    }
    
    try {
        if (j.contains("ProjectName")) {
            if (j["ProjectName"].is_null()) {
                throw json::type_error::create(302, "字段 'ProjectName' 为null，期望string类型", &j);
            }
            ProjectName = j["ProjectName"].get<std::string>();
        } else if (j.contains("ProjectInfo") && j["ProjectInfo"].is_object()) {
            const auto& projectInfo = j["ProjectInfo"];
            if (projectInfo.contains("ProjectName")) {
                if (projectInfo["ProjectName"].is_null()) {
                    throw json::type_error::create(302, "字段 'ProjectInfo.ProjectName' 为null，期望string类型", &j);
                }
                ProjectName = projectInfo["ProjectName"].get<std::string>();
            }
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'ProjectName': " + std::string(e.what()), &j);
    }
    
    // GeometryPath 可能在 ProjectInfo 中
    try {
        if (j.contains("GeometryPath")) {
            if (!j["GeometryPath"].is_null()) {
                GeometryPath = j["GeometryPath"].get<std::string>();
            }
        } else if (j.contains("ProjectInfo") && j["ProjectInfo"].is_object()) {
            const auto& projectInfo = j["ProjectInfo"];
            if (projectInfo.contains("GeometryPath")) {
                if (!projectInfo["GeometryPath"].is_null()) {
                    GeometryPath = projectInfo["GeometryPath"].get<std::string>();
                }
            }
        }
    } catch (const json::type_error& e) {
        throw json::type_error::create(302, "字段 'GeometryPath': " + std::string(e.what()), &j);
    }
    
    // OriginalGeometryPath 已移除，不再加载
    
    // FluidMeshInfo
    if (j.contains("FluidMeshInfo") && j["FluidMeshInfo"].is_object()) {
        FluidMeshInfo.fromJson(j["FluidMeshInfo"]);
    }
    
    // 清理现有数据
    ModelTree.clear();
    SetList.clear();
    AllSolidList.clear();
    
    // ModelTree
    if (j.contains("ModelTree") && j["ModelTree"].is_array()) {
        for (const auto& itemJson : j["ModelTree"]) {
            auto item = std::make_shared<ModelItem>();
            if (itemJson.contains("Id")) item->Id = itemJson["Id"].get<int>();
            if (itemJson.contains("Index")) item->Index = itemJson["Index"].get<int>();
            if (itemJson.contains("Path")) item->Path = itemJson["Path"].get<std::string>();
            if (itemJson.contains("BaryCenter")) {
                auto bc = itemJson["BaryCenter"];
                item->BaryCenter.x = bc.value("X", 0.0);
                item->BaryCenter.y = bc.value("Y", 0.0);
                item->BaryCenter.z = bc.value("Z", 0.0);
            }
            if (itemJson.contains("Volume")) item->Volume = itemJson["Volume"].get<double>();
            ModelTree.push_back(item);
        }
    }
    
    // SetList
    if (j.contains("SetList") && j["SetList"].is_array()) {
        for (const auto& setJson : j["SetList"]) {
            std::string setType = setJson.value("SetType", "");
            std::shared_ptr<SetItemBase> set;
            
            if (setType == "Solid") {
                auto solidSet = std::make_shared<SetSolidItem>();
                solidSet->fromJson(setJson);
                set = solidSet;
            } else if (setType == "Face") {
                auto faceSet = std::make_shared<SetFaceItem>();
                faceSet->fromJson(setJson);
                set = faceSet;
            } else {
                // 默认创建 Solid 集合
                auto solidSet = std::make_shared<SetSolidItem>();
                solidSet->fromJson(setJson);
                set = solidSet;
            }
            
            if (set) {
                SetList.push_back(set);
            }
        }
    }
    
    // AllSolidList
    if (j.contains("AllSolidList") && j["AllSolidList"].is_array()) {
        for (const auto& itemJson : j["AllSolidList"]) {
            SolidItem item;
            item.fromJson(itemJson);
            AllSolidList.push_back(item);
        }
    }
}

bool ProjectModelData::loadFromJsonFile(const std::string& filePath)
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filePath << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        file.close();
        
        fromJson(j);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "读取JSON文件失败: " << e.what() << std::endl;
        return false;
    }
}

bool ProjectModelData::saveToJsonFile(const std::string& filePath) const
{
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建文件: " << filePath << std::endl;
            return false;
        }
        
        json j = toJson();
        file << j.dump(4); // 格式化输出，缩进4个空格
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存JSON文件失败: " << e.what() << std::endl;
        return false;
    }
}

void ProjectModelData::reset()
{
    Name.reset();
    Description.reset();
    IsEnabled.reset();
    WorkingDirectory.reset();
    ProjectName.reset();
    GeometryPath.reset();
    OriginalGeometryPath.reset();
    ModelTree.clear();
    SetList.clear();
    AllSolidList.clear();
    VolumeRenameMap.clear();
}

void ProjectModelData::addVolumeRename(const std::string& oldName, const std::string& newName)
{
    VolumeRenameMap.emplace_back(oldName, newName);
}

std::string ProjectModelData::mapNewVolumeNameToOld(const std::string& newName) const
{
    for (const auto& pair : VolumeRenameMap)
    {
        if (pair.second == newName)
        {
            return pair.first;
        }
    }
    return newName;
}

std::set<std::string> ProjectModelData::getNonWallBoundaryConditionNames() const
{
    std::set<std::string> nonWallNames;
    
    // 从JSON中读取BoundaryConditionInfo（如果已加载）
    // 注意：这里我们需要访问原始JSON数据，但由于fromJson后没有保存原始JSON，
    // 我们需要在fromJson时保存BoundaryConditionInfo，或者通过其他方式获取
    
    // 暂时返回空集合，实际实现需要在fromJson时保存BoundaryConditionInfo
    return nonWallNames;
}

void ProjectModelData::AddModelItem(std::shared_ptr<ModelItem> item)
{
    if (item) {
        ModelTree.push_back(item);
    }
}

void ProjectModelData::AddSetItem(std::shared_ptr<SetItemBase> set)
{
    if (set) {
        SetList.push_back(set);
    }
}

void ProjectModelData::AddSolidItem(const SolidItem& item)
{
    AllSolidList.push_back(item);
}

void ProjectModelData::printInfo() const
{
    std::cout << "\n========== ProjectModelData 信息 ==========" << std::endl;
    std::cout << "名称: " << getName() << std::endl;
    std::cout << "描述: " << getDescription() << std::endl;
    std::cout << "启用状态: " << (getIsEnabled() ? "是" : "否") << std::endl;
    std::cout << "ModelTree 数量: " << ModelTree.size() << std::endl;
    std::cout << "SetList 数量: " << SetList.size() << std::endl;
    std::cout << "AllSolidList 数量: " << AllSolidList.size() << std::endl;
    std::cout << "================================================" << std::endl;
}

void ProjectModelData::printSummary() const
{
    printInfo();
    
    if (!ModelTree.empty()) {
        std::cout << "\nModelTree 详情:" << std::endl;
        for (size_t i = 0; i < ModelTree.size(); ++i) {
            const auto& item = ModelTree[i];
            std::cout << "  [" << i << "] ID=" << item->Id 
                      << ", Path=" << item->Path 
                      << ", Volume=" << item->Volume << std::endl;
        }
    }
    
    if (!SetList.empty()) {
        std::cout << "\nSetList 详情:" << std::endl;
        for (size_t i = 0; i < SetList.size(); ++i) {
            const auto& set = SetList[i];
            std::cout << "  [" << i << "] SetName=" 
                      << (set->SetName.has_value() ? set->SetName.value() : "N/A")
                      << ", SetType=" 
                      << (set->SetType.has_value() ? set->SetType.value() : "N/A") << std::endl;
        }
    }
    
    if (!AllSolidList.empty()) {
        std::cout << "\nAllSolidList 详情:" << std::endl;
        for (size_t i = 0; i < AllSolidList.size(); ++i) {
            const auto& item = AllSolidList[i];
            std::cout << "  [" << i << "] ID=" << item.Id 
                      << ", Path=" << item.Path 
                      << ", Volume=" << item.Volume << std::endl;
        }
    }
}

