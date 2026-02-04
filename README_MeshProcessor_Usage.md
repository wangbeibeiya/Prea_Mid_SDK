# MeshProcessor 使用说明 - 自动边界层设置

## 概述

`MeshProcessor` 现在支持在设置完全局网格参数后自动设置边界层。当 `MeshParameters` 中包含边界层参数时，`setGlobalParameters` 方法会在设置完全局参数后自动调用边界层设置。

## 使用方法

### 方法1：在全局参数中直接包含边界层参数（推荐）

```cpp
#include "MeshProcessor.h"
#include "ProjectModelData.h"
#include <json.hpp>

// 创建MeshProcessor
MeshProcessor meshProcessor(pfDocument);
meshProcessor.initialize();

// 从JSON加载数据
nlohmann::json jsonData;
// ... 加载JSON数据 ...

// 创建网格参数
MeshProcessor::MeshParameters meshParams;
meshParams.minSize = 0.01;
meshParams.maxSize = 0.1;
meshParams.growthRate = 1.2;
// ... 设置其他全局参数 ...

// 如果JSON中包含边界层信息，添加到网格参数中
if (jsonData.contains("BoundaryLayersInfo")) {
    const auto& blInfo = jsonData["BoundaryLayersInfo"];
    
    MeshProcessor::BoundaryLayerParameters blParams;
    blParams.firstLayerHeight = blInfo.value("FirstLayerHeight", 0.001);
    blParams.growthRate = blInfo.value("GrowthRate", 1.2);
    blParams.layersNumber = blInfo.value("LayersNumber", 5);
    blParams.isBoundaryLayers = blInfo.value("IsBoundaryLayers", true);
    
    // 将边界层参数添加到网格参数中
    meshParams.boundaryLayerParams = blParams;
    meshParams.fluidZoneSetName = blInfo.value("FluidZoneSet", "");
}

// 加载ProjectModelData（用于边界层设置）
ProjectModelData modelData;
modelData.fromJson(jsonData);

// 设置全局参数（会自动设置边界层）
if (!meshProcessor.setGlobalParameters(meshParams, &modelData)) {
    std::cerr << "设置失败: " << meshProcessor.getLastError() << std::endl;
    return;
}

// 现在可以生成网格了
meshProcessor.createSurfaceMesh();
meshProcessor.createVolumeMeshByGeometry(true, false);  // withInflation=true 启用边界层
```

### 方法2：分别设置全局参数和边界层

```cpp
// 1. 设置全局参数（不包含边界层）
MeshProcessor::MeshParameters meshParams;
meshParams.minSize = 0.01;
meshParams.maxSize = 0.1;
// ... 设置其他参数 ...
meshProcessor.setGlobalParameters(meshParams);

// 2. 单独设置边界层
if (jsonData.contains("BoundaryLayersInfo")) {
    const auto& blInfo = jsonData["BoundaryLayersInfo"];
    
    MeshProcessor::BoundaryLayerParameters blParams;
    blParams.firstLayerHeight = blInfo["FirstLayerHeight"];
    blParams.growthRate = blInfo["GrowthRate"];
    blParams.layersNumber = blInfo["LayersNumber"];
    blParams.isBoundaryLayers = blInfo["IsBoundaryLayers"];
    
    std::string fluidZoneSet = blInfo["FluidZoneSet"];
    ProjectModelData modelData;
    modelData.fromJson(jsonData);
    
    meshProcessor.setBoundaryLayersByFluidZoneSet(fluidZoneSet, blParams, &modelData);
}
```

## 工作流程

当调用 `setGlobalParameters` 时：

1. **设置全局网格参数**：
   - 网格尺寸（minSize, maxSize）
   - 增长率（growthRate）
   - 曲率法向角度（curvatureNormalAngle）
   - 邻近检测设置
   - 间隙设置
   - 网格质量优化设置
   - 膨胀相关全局参数

2. **自动设置边界层**（如果提供了边界层参数）：
   - 检查 `parameters.boundaryLayerParams` 和 `parameters.fluidZoneSetName` 是否有值
   - 如果有，自动调用 `setBoundaryLayersByFluidZoneSet`
   - 根据 `FluidZoneSet` 从 `SetList` 中找到对应的体对象
   - 对每个匹配体对象的所有组设置边界层参数

## 注意事项

1. **边界层设置时机**：
   - 边界层必须在网格划分**之前**设置
   - 如果使用自动设置，确保在调用 `createSurfaceMesh` 或 `createVolumeMeshByGeometry` 之前调用 `setGlobalParameters`

2. **参数完整性**：
   - 要启用自动边界层设置，必须同时提供 `boundaryLayerParams` 和 `fluidZoneSetName`
   - 如果只提供了其中一个，不会自动设置边界层

3. **错误处理**：
   - 如果边界层设置失败，不会影响全局参数设置的成功
   - 会输出警告信息，但 `setGlobalParameters` 仍然返回 `true`（因为全局参数已成功设置）

4. **ProjectModelData**：
   - 如果提供了 `modelData`，边界层设置会根据 `SetList` 精确匹配体对象
   - 如果不提供 `modelData`，会对所有体对象设置边界层（可能不是期望的行为）

## 示例JSON配置

```json
{
    "FluidMeshInfo": {
        "GrowthRate": 1.2,
        "MaxMeshSize": 10,
        "MinMeshSize": 2,
        "NormalAngle": 18
    },
    "BoundaryLayersInfo": {
        "FirstLayerHeight": 0.001,
        "FluidZoneSet": "LT",
        "GrowthRate": 1.2,
        "IsBoundaryLayers": true,
        "LayersNumber": 5
    },
    "SetList": [
        {
            "SetName": "LT",
            "SetType": "Solid",
            "Items": [
                {
                    "Id": 1366526861,
                    "Path": "all-workbench",
                    "Volume": 3.9287172413803725e-05
                }
            ]
        }
    ]
}
```

## 完整示例

```cpp
// 从JSON文件加载配置
nlohmann::json jsonData;
std::ifstream file("config.json");
file >> jsonData;

// 创建处理器
MeshProcessor meshProcessor(pfDocument);
meshProcessor.initialize();

// 准备网格参数
MeshProcessor::MeshParameters meshParams;

// 从FluidMeshInfo设置全局参数
if (jsonData.contains("FluidMeshInfo")) {
    const auto& meshInfo = jsonData["FluidMeshInfo"];
    meshParams.minSize = meshInfo.value("MinMeshSize", 0.01);
    meshParams.maxSize = meshInfo.value("MaxMeshSize", 0.1);
    meshParams.growthRate = meshInfo.value("GrowthRate", 1.2);
    meshParams.curvatureNormalAngle = meshInfo.value("NormalAngle", 30.0);
}

// 从BoundaryLayersInfo设置边界层参数
if (jsonData.contains("BoundaryLayersInfo")) {
    const auto& blInfo = jsonData["BoundaryLayersInfo"];
    
    MeshProcessor::BoundaryLayerParameters blParams;
    blParams.firstLayerHeight = blInfo.value("FirstLayerHeight", 0.001);
    blParams.growthRate = blInfo.value("GrowthRate", 1.2);
    blParams.layersNumber = blInfo.value("LayersNumber", 5);
    blParams.isBoundaryLayers = blInfo.value("IsBoundaryLayers", true);
    
    meshParams.boundaryLayerParams = blParams;
    meshParams.fluidZoneSetName = blInfo.value("FluidZoneSet", "");
}

// 加载项目模型数据
ProjectModelData modelData;
modelData.fromJson(jsonData);

// 一次性设置全局参数和边界层
if (meshProcessor.setGlobalParameters(meshParams, &modelData)) {
    // 生成网格
    meshProcessor.createSurfaceMesh();
    meshProcessor.createVolumeMeshByGeometry(true, false);  // 启用边界层
} else {
    std::cerr << "设置失败: " << meshProcessor.getLastError() << std::endl;
}
```

## 优势

使用自动边界层设置的优势：

1. **简化工作流程**：一次调用完成全局参数和边界层设置
2. **减少错误**：确保边界层在正确的时机设置
3. **代码更清晰**：所有网格相关参数集中在一个结构体中
4. **向后兼容**：如果不提供边界层参数，行为与之前相同
