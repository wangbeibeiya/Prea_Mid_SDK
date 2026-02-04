// Revision: $Id: GeometryAPIExample.cpp,v 1.0 2025/02/27 10:30:00 assistant Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2021] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technoglies, Inc.
 * The intellectual and technical concepts contained
 * herein are proprietary to PeraGlobal Technologies, Inc.
 * and may be covered by China. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from PeraGlobal Technologies, Inc.
 */

#include "GeometryAPI.h"
#include <iostream>
#include <vector>

/**
 * @brief GeometryAPI使用示例
 * 
 * 本文件展示了如何使用GeometryAPI进行各种几何操作
 */

void demonstrateBasicOperations()
{
    std::cout << "\n=== GeometryAPI 基本操作示例 ===" << std::endl;
    
    // 创建GeometryAPI实例
    GeometryAPI api("E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/");
    
    // 初始化API
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    std::cout << "GeometryAPI初始化成功！" << std::endl;
    
    // 导入几何文件
    std::string filePath = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/quickRepair.ppcf";
    if (api.importGeometry(filePath))
    {
        std::cout << "几何文件导入成功！" << std::endl;
        
        // 执行快速修复
        if (api.quickRepair(1e-5))
        {
            std::cout << "快速修复完成！" << std::endl;
        }
        
        // 查找体积
        int volumeCount = api.findVolumes();
        if (volumeCount >= 0)
        {
            std::cout << "找到 " << volumeCount << " 个体积" << std::endl;
        }
        
        // 显示几何
        api.showGeometry();
    }
    else
    {
        std::cerr << "几何文件导入失败: " << api.getLastError() << std::endl;
    }
}

void demonstrateGeometryCreation()
{
    std::cout << "\n=== GeometryAPI 几何创建示例 ===" << std::endl;
    
    GeometryAPI api;
    
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    // 创建立方体
    std::vector<float> startPoint = {0.0f, 0.0f, 0.0f};
    std::vector<float> endPoint = {1.0f, 1.0f, 1.0f};
    
    if (api.createCube("TestCube", startPoint, endPoint))
    {
        std::cout << "立方体创建成功！" << std::endl;
    }
    
    // 创建球体
    std::vector<float> center = {2.0f, 0.0f, 0.0f};
    float radius = 0.5f;
    
    if (api.createSphere("TestSphere", center, radius))
    {
        std::cout << "球体创建成功！" << std::endl;
    }
    
    // 创建圆柱体
    std::vector<float> cylStart = {0.0f, 2.0f, 0.0f};
    std::vector<float> cylEnd = {1.0f, 2.0f, 0.0f};
    
    if (api.createCylinder("TestCylinder", cylStart, cylEnd, 0.3f))
    {
        std::cout << "圆柱体创建成功！" << std::endl;
    }
    
    // 显示创建的几何
    api.showGeometry();
}

void demonstrateGeometryTransformation()
{
    std::cout << "\n=== GeometryAPI 几何变换示例 ===" << std::endl;
    
    GeometryAPI api;
    
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    // 先创建一个立方体用于变换
    std::vector<float> startPoint = {0.0f, 0.0f, 0.0f};
    std::vector<float> endPoint = {1.0f, 1.0f, 1.0f};
    
    if (!api.createCube("TransformCube", startPoint, endPoint))
    {
        std::cerr << "立方体创建失败" << std::endl;
        return;
    }
    
    // 假设立方体的实体ID为1（实际应用中需要从API获取）
    std::vector<unsigned int> entityIds = {1};
    
    // 缩放操作
    std::vector<double> origin = {0.0, 0.0, 0.0};
    if (api.scale(entityIds, origin, 2.0))
    {
        std::cout << "缩放操作成功！" << std::endl;
    }
    
    // 平移操作
    std::vector<double> offset = {2.0, 0.0, 0.0};
    if (api.translate(entityIds, offset, true))  // 复制并变换
    {
        std::cout << "平移操作成功！" << std::endl;
    }
    
    // 旋转操作
    std::vector<double> rotateOrigin = {0.0, 0.0, 0.0};
    std::vector<double> axis = {0.0, 0.0, 1.0};  // 绕Z轴旋转
    if (api.rotate(entityIds, rotateOrigin, axis, 45.0))  // 旋转45度
    {
        std::cout << "旋转操作成功！" << std::endl;
    }
    
    // 镜像操作
    std::vector<double> mirrorOrigin = {0.0, 0.0, 0.0};
    std::vector<double> mirrorDirection = {1.0, 0.0, 0.0};  // 沿X轴镜像
    if (api.mirror(entityIds, mirrorOrigin, mirrorDirection, true))  // 复制并镜像
    {
        std::cout << "镜像操作成功！" << std::endl;
    }
    
    // 显示变换后的几何
    api.showGeometry();
}

void demonstrateGeometryRepair()
{
    std::cout << "\n=== GeometryAPI 几何修复示例 ===" << std::endl;
    
    GeometryAPI api;
    
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    // 导入需要修复的几何文件
    std::string repairFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/stitch.stp";
    if (api.importGeometry(repairFile))
    {
        std::cout << "导入几何文件成功，开始修复操作..." << std::endl;
        
        // 执行快速修复
        if (api.quickRepair(1e-5))
        {
            std::cout << "快速修复完成！" << std::endl;
        }
        
        // 查找体积
        int volumeCount = api.findVolumes();
        std::cout << "修复后找到 " << volumeCount << " 个体积" << std::endl;
        
        // 假设有两个相邻的面需要缝合（实际ID需要从几何数据获取）
        if (api.stitchTwoFaces(17, 18, 0.001))
        {
            std::cout << "面缝合成功！" << std::endl;
        }
        
        // 填充所有孔洞
        if (api.fillAllHoles())
        {
            std::cout << "孔洞填充完成！" << std::endl;
        }
        
        // 去除小特征
        if (api.defeature(1e-6))
        {
            std::cout << "特征去除完成！" << std::endl;
        }
        
        // 显示修复后的几何
        api.showGeometry();
    }
    else
    {
        std::cerr << "导入几何文件失败: " << api.getLastError() << std::endl;
    }
}

void demonstrateVolumeOperations()
{
    std::cout << "\n=== GeometryAPI 体积操作示例 ===" << std::endl;
    
    GeometryAPI api;
    
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    // 导入包含面组的几何文件
    std::string volumeFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/createVolume.ppcf";
    if (api.importGeometry(volumeFile))
    {
        std::cout << "导入几何文件成功，开始体积操作..." << std::endl;
        
        // 从面组创建体积
        std::vector<std::string> groupNames = {"b2"};
        if (api.createVolume(groupNames))
        {
            std::cout << "体积创建成功！" << std::endl;
        }
        
        // 重命名体积
        if (api.renameVolume("Volume1", "MainVolume"))
        {
            std::cout << "体积重命名成功！" << std::endl;
        }
        
        // 重命名面组
        if (api.renameFaceGroup("b2_unused", "b2_renamed"))
        {
            std::cout << "面组重命名成功！" << std::endl;
        }
        
        // 分离体积
        std::vector<std::string> volumeNames = {"MainVolume"};
        if (api.detachVolumes(volumeNames))
        {
            std::cout << "体积分离成功！" << std::endl;
        }
        
        // 显示最终结果
        api.showGeometry();
    }
    else
    {
        std::cerr << "导入几何文件失败: " << api.getLastError() << std::endl;
    }
}

void demonstrateMeshToGeometry()
{
    std::cout << "\n=== GeometryAPI 网格转几何示例 ===" << std::endl;
    
    GeometryAPI api;
    
    if (!api.initialize())
    {
        std::cerr << "初始化失败: " << api.getLastError() << std::endl;
        return;
    }
    
    // 导入网格文件并转换为几何
    std::string meshFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/convertMeshToGeometry.msh";
    if (api.importMeshAndConvertToGeometry(meshFile))
    {
        std::cout << "网格转几何成功！" << std::endl;
        
        // 在转换后的几何上执行修复操作
        api.quickRepair();
        
        int volumeCount = api.findVolumes();
        std::cout << "转换后找到 " << volumeCount << " 个体积" << std::endl;
        
        // 显示转换后的几何
        api.showGeometry();
    }
    else
    {
        std::cerr << "网格转几何失败: " << api.getLastError() << std::endl;
    }
}

/**
 * @brief 主函数 - 演示GeometryAPI的各种功能
 */
int main()
{
    std::cout << "=== GeometryAPI 使用示例程序 ===" << std::endl;
    std::cout << "本程序演示如何使用GeometryAPI进行各种几何操作" << std::endl;
    
    try
    {
        // 基本操作示例
        demonstrateBasicOperations();
        
        // 几何创建示例
        demonstrateGeometryCreation();
        
        // 几何变换示例
        demonstrateGeometryTransformation();
        
        // 几何修复示例
        demonstrateGeometryRepair();
        
        // 体积操作示例
        demonstrateVolumeOperations();
        
        // 网格转几何示例
        demonstrateMeshToGeometry();
        
        std::cout << "\n=== 所有示例执行完成 ===" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "程序执行过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
