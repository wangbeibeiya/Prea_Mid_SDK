# 边界层参数设置使用说明

## 概述

本功能实现了从JSON配置中加载边界层参数，并根据`FluidZoneSet`指定的集合名称，对相应体对象中的所有组设置边界层参数。

## JSON配置格式

边界层参数在JSON中的格式如下：

```json
"BoundaryLayersInfo": {
    "FirstLayerHeight": 0.001,
    "FluidZoneSet": "LT",
    "GrowthRate": 1.2,
    "IsBoundaryLayers": true,
    "LayersNumber": 5
}
```

### 参数说明

- `FirstLayerHeight`: 第一层边界层高度
- `FluidZoneSet`: 流体区域集合名称（对应SetList中的SetName）
- `GrowthRate`: 边界层高度增长率
- `IsBoundaryLayers`: 是否启用边界层
- `LayersNumber`: 边界层数量

## 使用方法

### 1. 基本使用

```cpp
#include "MeshProcessor.h"
#include "ProjectModelData.h"
#include <json.hpp>

// 创建MeshProcessor
MeshProcessor meshProcessor(pfDocument);
if (!meshProcessor.initialize()) {
    std::cerr << "初始化失败: " << meshProcessor.getLastError() << std::endl;
    return;
}

// 从JSON加载边界层参数
nlohmann::json jsonData = /* ... 从文件或字符串加载JSON ... */;
if (jsonData.contains("BoundaryLayersInfo")) {
    const auto& blInfo = jsonData["BoundaryLayersInfo"];
    
    // 创建边界层参数结构
    MeshProcessor::BoundaryLayerParameters blParams;
    blParams.firstLayerHeight = blInfo.value("FirstLayerHeight", 0.001);
    blParams.growthRate = blInfo.value("GrowthRate", 1.2);
    blParams.layersNumber = blInfo.value("LayersNumber", 5);
    blParams.isBoundaryLayers = blInfo.value("IsBoundaryLayers", true);
    
    // 获取FluidZoneSet名称
    std::string fluidZoneSet = blInfo.value("FluidZoneSet", "");
    
    // 加载ProjectModelData（包含SetList信息）
    ProjectModelData modelData;
    modelData.fromJson(jsonData);
    
    // 设置边界层参数
    if (!meshProcessor.setBoundaryLayersByFluidZoneSet(fluidZoneSet, blParams, &modelData)) {
        std::cerr << "设置边界层失败: " << meshProcessor.getLastError() << std::endl;
        return;
    }
}
```

### 2. 完整工作流程

```cpp
// 1. 初始化MeshProcessor
MeshProcessor meshProcessor(pfDocument);
meshProcessor.initialize();

// 2. 设置全局网格参数（如果需要）
MeshProcessor::MeshParameters meshParams;
meshParams.minSize = 0.01;
meshParams.maxSize = 0.1;
meshProcessor.setGlobalParameters(meshParams);

// 3. 从JSON加载并设置边界层参数
nlohmann::json jsonData;
// ... 加载JSON数据 ...
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
    
    // 在网格划分前设置边界层
    meshProcessor.setBoundaryLayersByFluidZoneSet(fluidZoneSet, blParams, &modelData);
}

// 4. 生成表面网格
meshProcessor.createSurfaceMesh();

// 5. 生成体积网格（启用边界层）
meshProcessor.createVolumeMeshByGeometry(true, false);
```

## 工作原理

1. **查找匹配的体对象**：
   - 如果提供了`ProjectModelData`，函数会从`SetList`中查找`SetName`等于`FluidZoneSet`的集合
   - 获取该集合中所有`SolidItem`的`Id`
   - 在几何数据中查找匹配这些ID的体对象

2. **设置边界层参数**：
   - 对每个匹配的体对象，遍历其所有组（group）
   - 跳过系统组（如`.nodes`, `.edges`等）
   - 对每个有效组设置边界层参数：
     - 开启边界层：`setInflationOn(volumeName, groupName, true)`
     - 设置第一层高度：`setInitialHeight(volumeName, groupName, firstLayerHeight)`
     - 设置层数：`setLayerNumber(volumeName, groupName, layersNumber)`
     - 设置增长率：`setHeightRatio(volumeName, groupName, growthRate)`

## 注意事项

1. **必须在网格划分前设置**：边界层参数需要在调用`createVolumeMeshByGeometry`或`createVolumeMeshBySurfaceMesh`之前设置

2. **FluidZoneSet匹配**：
   - `FluidZoneSet`必须与`SetList`中某个集合的`SetName`完全匹配
   - 该集合的类型必须是`"Solid"`

3. **体对象匹配**：
   - 如果提供了`ProjectModelData`，函数会根据集合中`SolidItem`的`Id`来匹配体对象
   - 如果没有提供`ProjectModelData`，函数会对所有体对象的所有组设置边界层（可能不是期望的行为）

4. **组过滤**：
   - 函数会自动跳过系统组（名称包含"node"或"edge"，或以"."开头）
   - 只对有效的几何组设置边界层

## 错误处理

函数会返回`bool`值表示是否成功：
- `true`: 成功设置边界层
- `false`: 设置失败，可通过`getLastError()`获取错误信息

常见的错误情况：
- PFMesh未初始化
- 无法获取几何环境
- 未找到体对象
- 未找到匹配的体对象或组
- 边界层参数设置失败（会输出警告但继续处理其他组）

## 示例JSON

```json
{
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

在这个例子中，函数会：
1. 查找`SetName`为"LT"的集合
2. 获取该集合中`Id`为1366526861的体对象
3. 对该体对象中的所有有效组设置边界层参数
