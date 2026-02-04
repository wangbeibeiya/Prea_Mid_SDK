# Pera CFD SDK 模型导入器

## 项目概述

本项目基于Pera CFD SDK实现了一个模型导入器，用于导入几何模型、网格文件和后处理结果。项目已经从原来的VTK 3D可视化示例转换为基于第三方库的模型导入功能。

## 主要功能

### 1. 几何模型导入
- 支持多种CAD格式：STL、STEP、IGES、OBJ、PLY
- 支持追加和替换两种导入模式
- 自动设置导入选项和分组模式

### 2. 网格文件导入
- 支持多种网格格式：CGNS、FLUENT、STAR、VTK
- 自动处理网格数据加载
- 提供导入状态反馈

### 3. 后处理结果导入
- 支持CGNS等后处理结果格式
- 自动加载计算结果数据
- 提供加载时间统计

### 4. 模型显示
- 获取几何数据
- 获取网格数据
- 为后续渲染提供数据接口

### 5. 许可证管理
- 自动检查Pera CFD SDK许可证
- 验证各模块权限
- 提供详细的错误信息

## 文件结构

```
Prea_Mid_SDK/
├── include/
│   └── ModelImporter.h          # 模型导入器头文件
├── src/
│   ├── ModelImporter.cpp        # 模型导入器实现
│   └── main.cpp                 # 主程序
├── tests/
│   ├── test_model_importer.cpp  # 测试程序
│   └── CMakeLists.txt           # 测试配置
├── demo/                        # 原始demo文件夹（参考）
├── CMakeLists.txt               # 主CMake配置
└── README.md                    # 项目说明
```

## 编译说明

### 环境要求
- Visual Studio 2019或更高版本
- CMake 3.16或更高版本
- Pera CFD SDK（路径：E:/wbb/GitSrc/Pera_CFD_SDK/pfsdk_0704/pfsdk/0704）

### 编译步骤
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译项目
cmake --build . --config Debug
```

### 运行程序
```bash
# 运行主程序
.\bin\Debug\VTKProject.exe

# 运行测试程序
.\tests\Debug\test_model_importer.exe
```

## 编码问题解决方案

### 问题描述
在Windows环境下，中文注释可能导致编译错误：
- 警告：`warning C4819: 该文件包含不能在当前代码页(936)中表示的字符`
- 错误：`error C2001: 常量中有换行符`

### 解决方案
在CMakeLists.txt中添加UTF-8编码支持：

```cmake
# 设置UTF-8编码支持
if(MSVC)
    target_compile_options(ModelImporter PRIVATE /utf-8)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()
```

### 验证方法
编译成功后，程序应该能够正确处理中文注释和输出。

## 技术特点

### 1. 面向对象设计
- 使用C++类封装SDK功能
- 提供清晰的接口和错误处理
- 支持资源自动管理

### 2. 异常处理
- 全面的try-catch异常处理
- 详细的错误信息输出
- 优雅的错误恢复机制

### 3. 性能优化
- 使用智能指针管理内存
- 提供导入时间统计
- 支持异步操作

### 4. 可扩展性
- 模块化设计
- 易于添加新的文件格式支持
- 支持插件式架构

## 使用示例

```cpp
#include "ModelImporter.h"

int main() {
    // 创建模型导入器
    ModelImporter importer;
    
    // 初始化SDK
    if (!importer.initialize()) {
        std::cerr << "SDK初始化失败" << std::endl;
        return 1;
    }
    
    // 导入几何模型
    if (importer.importGeometry("model.stl", 0)) {
        std::cout << "几何模型导入成功" << std::endl;
    }
    
    // 导入网格文件
    if (importer.importMesh("mesh.cgns")) {
        std::cout << "网格文件导入成功" << std::endl;
    }
    
    // 显示模型
    importer.showModel();
    
    return 0;
}
```

## 注意事项

1. **编码问题**：已通过CMake配置解决中文编码问题
2. **许可证要求**：需要有效的Pera CFD SDK许可证才能正常运行
3. **路径配置**：确保Pera CFD SDK路径正确配置在CMakeLists.txt中
4. **依赖库**：确保所有必要的库文件都在正确的位置

## 编译状态

✅ **编译成功**：项目已成功编译，包括：
- ModelImporter库
- 主程序（VTKProject.exe）
- 测试程序（test_model_importer.exe）

✅ **编码问题已解决**：通过添加`/utf-8`编译选项解决了中文注释问题

## 未来改进

1. 添加更多文件格式支持
2. 实现模型渲染功能
3. 添加批量导入功能
4. 优化性能和内存使用
5. 添加配置文件支持
6. 实现多线程导入

## 许可证

本项目基于Pera CFD SDK开发，请遵守相应的许可证条款。 