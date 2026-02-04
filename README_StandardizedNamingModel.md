# StandardizedNamingModel 使用说明

## 概述

`StandardizedNamingModel` 类参考了 `co-de-sim` 项目中的 `StandardizedNamingModel`，用于管理几何模型的三个核心数据结构：

1. **ModelTree** - 模型树，存储所有模型项
2. **SetList** - 集合列表，存储各种类型的几何集合（实体、面等）
3. **AllSolidList** - 所有实体列表，存储所有实体项

## 数据结构说明

### ModelTree
- 类型：`std::vector<std::shared_ptr<ModelItem>>`
- 用途：存储模型的层次结构信息
- 每个 `ModelItem` 包含：
  - `Id` - 对象在CAD中的ID
  - `Index` - 在ModelTree中的索引
  - `Path` - 对象在CAD中的路径
  - `BaryCenter` - 重心坐标
  - `Volume` - 体积

### SetList
- 类型：`std::vector<std::shared_ptr<SetItemBase>>`
- 用途：存储各种类型的几何集合
- 支持的集合类型：
  - `SetSolidItem` - 实体集合
  - `SetFaceItem` - 面集合
- 每个集合包含：
  - `SetName` - 集合名称
  - `SetType` - 集合类型（"Solid"、"Face"等）
  - `Items` - 集合成员列表

### AllSolidList
- 类型：`std::vector<SolidItem>`
- 用途：存储所有实体项的列表
- 每个 `SolidItem` 包含完整的实体几何属性

## JSON 格式

程序支持从 JSON 文件读取和保存数据。JSON 格式示例：

```json
{
    "Name": "模型名称",
    "Description": "模型描述",
    "IsEnabled": true,
    "ModelTree": [
        {
            "Id": 1,
            "Index": 0,
            "Path": "/Model/Solid1",
            "BaryCenter": {"X": 0.0, "Y": 0.0, "Z": 0.0},
            "Volume": 1000.0
        }
    ],
    "SetList": [
        {
            "SetName": "实体集合1",
            "SetType": "Solid",
            "Items": [...]
        }
    ],
    "AllSolidList": [...]
}
```

完整示例请参考 `example_model.json` 文件。

## 使用方法

### 1. 从 JSON 文件加载数据

```bash
# 基本用法：加载 JSON 文件
VTKProject.exe example_model.json

# 加载并保存到新文件
VTKProject.exe example_model.json output.json
```

### 2. 在代码中使用

```cpp
#include "../include/StandardizedNamingModel.h"

// 创建模型对象
StandardizedNamingModel model;

// 从 JSON 文件加载
if (model.loadFromJsonFile("example_model.json")) {
    // 访问数据
    auto& modelTree = model.GetModelTree();
    auto& setList = model.GetSetList();
    auto& allSolidList = model.GetAllSolidList();
    
    // 打印信息
    model.printSummary();
    
    // 保存到文件
    model.saveToJsonFile("output.json");
}
```

### 3. 手动创建数据

```cpp
// 创建模型项
auto item = std::make_shared<ModelItem>();
item->Id = 1;
item->Index = 0;
item->Path = "/Model/Solid1";
item->BaryCenter = Point3D(0.0, 0.0, 0.0);
item->Volume = 1000.0;
model.AddModelItem(item);

// 创建实体集合
auto solidSet = std::make_shared<SetSolidItem>();
solidSet->SetName = "实体集合1";
SolidItem solidItem;
solidItem.Id = 1;
solidItem.Path = "/Model/Solid1";
solidItem.Volume = 1000.0;
solidSet->Items.push_back(solidItem);
model.AddSetItem(solidSet);

// 添加实体到 AllSolidList
SolidItem solid;
solid.Id = 1;
solid.Volume = 1000.0;
model.AddSolidItem(solid);
```

## 编译要求

- CMake 3.16 或更高版本
- C++17 标准
- nlohmann/json 库（通过 CMake FetchContent 自动下载）

## 注意事项

1. JSON 文件必须使用 UTF-8 编码
2. 所有坐标值使用双精度浮点数
3. 集合类型必须正确指定（"Solid"、"Face"等）
4. 如果 JSON 文件格式不正确，程序会输出错误信息

## 示例文件

项目根目录下的 `example_model.json` 文件包含了一个完整的示例，展示了所有三个数据结构的 JSON 格式。

