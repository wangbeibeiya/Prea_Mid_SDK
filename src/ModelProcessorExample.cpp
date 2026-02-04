// ModelProcessorExample.cpp - 演示新的模型处理器功能
#include "ModelProcessor.h"
#include <iostream>
#include <vector>
#include <string>

/**
 * @brief 演示基本模型处理功能
 */
void demonstrateBasicProcessing()
{
    std::cout << "\n=== 基本模型处理示例 ===" << std::endl;
    
    // 创建模型处理器
    ModelProcessor processor("E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/");
    
    // 初始化处理器
    if (!processor.initialize())
    {
        std::cerr << "处理器初始化失败: " << processor.getLastError() << std::endl;
        return;
    }
    
    // 设置处理选项
    ModelProcessor::ProcessOptions options;
    options.enableQuickRepair = true;
    options.enableFindVolumes = true;
    options.enableRendering = true;
    options.repairTolerance = 1e-5;
    
    // 处理几何模型
    std::string geometryFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/quickRepair.ppcf";
    if (processor.processGeometryModel(geometryFile, 1, options))
    {
        std::cout << "几何模型处理成功！" << std::endl;
    }
    else
    {
        std::cerr << "几何模型处理失败: " << processor.getLastError() << std::endl;
    }
    
    // 显示统计信息
    std::cout << processor.getProcessingStats() << std::endl;
}

/**
 * @brief 演示批量处理功能
 */
void demonstrateBatchProcessing()
{
    std::cout << "\n=== 批量模型处理示例 ===" << std::endl;
    
    ModelProcessor processor("E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/");
    
    if (!processor.initialize())
    {
        std::cerr << "处理器初始化失败: " << processor.getLastError() << std::endl;
        return;
    }
    
    // 准备文件列表
    std::vector<std::string> fileList = {
        "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/quickRepair.ppcf",
        "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/findVolumes.ppcf",
        "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/stitch.stp"
    };
    
    // 设置批量处理选项
    ModelProcessor::ProcessOptions batchOptions;
    batchOptions.enableQuickRepair = true;
    batchOptions.enableFindVolumes = true;
    batchOptions.enableRendering = false; // 批量处理时禁用渲染以提高速度
    
    // 执行批量处理
    int successCount = processor.batchProcessGeometry(fileList, 1, batchOptions);
    std::cout << "批量处理完成，成功处理 " << successCount << " 个文件" << std::endl;
    
    // 显示最终统计
    std::cout << processor.getProcessingStats() << std::endl;
}

/**
 * @brief 演示自定义处理选项
 */
void demonstrateCustomOptions()
{
    std::cout << "\n=== 自定义处理选项示例 ===" << std::endl;
    
    ModelProcessor processor;
    
    if (!processor.initialize())
    {
        std::cerr << "处理器初始化失败: " << processor.getLastError() << std::endl;
        return;
    }
    
    // 创建自定义处理选项
    ModelProcessor::ProcessOptions customOptions;
    customOptions.enableQuickRepair = true;
    customOptions.enableFindVolumes = false;  // 禁用体积查找
    customOptions.enableRendering = true;
    customOptions.repairTolerance = 1e-6;     // 更高精度的修复容差
    
    // 设置为默认选项
    processor.setDefaultProcessOptions(customOptions);
    
    // 处理模型（使用自定义选项）
    std::string modelFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/defeature.igs";
    if (processor.processGeometryModel(modelFile))
    {
        std::cout << "使用自定义选项处理成功！" << std::endl;
    }
    
    // 显示当前处理选项
    auto currentOptions = processor.getCurrentProcessOptions();
    std::cout << "当前处理选项:" << std::endl;
    std::cout << "  - 快速修复: " << (currentOptions.enableQuickRepair ? "启用" : "禁用") << std::endl;
    std::cout << "  - 体积查找: " << (currentOptions.enableFindVolumes ? "启用" : "禁用") << std::endl;
    std::cout << "  - 渲染显示: " << (currentOptions.enableRendering ? "启用" : "禁用") << std::endl;
    std::cout << "  - 修复容差: " << currentOptions.repairTolerance << std::endl;
}

/**
 * @brief 演示网格处理功能
 */
void demonstrateMeshProcessing()
{
    std::cout << "\n=== 网格处理示例 ===" << std::endl;
    
    ModelProcessor processor;
    
    if (!processor.initialize())
    {
        std::cerr << "处理器初始化失败: " << processor.getLastError() << std::endl;
        return;
    }
    
    // 处理网格文件
    std::string meshFile = "E:/wbb/GitSrc/Pera_CFD_SDK/PFSDK_20251103/examples/geometry/convertMeshToGeometry.msh";
    if (processor.processMeshModel(meshFile, true)) // 启用渲染
    {
        std::cout << "网格文件处理成功！" << std::endl;
    }
    else
    {
        std::cerr << "网格文件处理失败: " << processor.getLastError() << std::endl;
    }
}

/**
 * @brief 演示几何API的直接使用
 */
void demonstrateDirectAPIUsage()
{
    std::cout << "\n=== 直接使用几何API示例 ===" << std::endl;
    
    ModelProcessor processor;
    
    if (!processor.initialize())
    {
        std::cerr << "处理器初始化失败: " << processor.getLastError() << std::endl;
        return;
    }
    
    // 获取几何API实例
    GeometryAPI* api = processor.getGeometryAPI();
    
    // 直接使用API创建几何
    std::vector<float> start = {0.0f, 0.0f, 0.0f};
    std::vector<float> end = {1.0f, 1.0f, 1.0f};
    
    if (api->createCube("TestCube", start, end))
    {
        std::cout << "立方体创建成功！" << std::endl;
        
        // 执行一些几何操作
        std::vector<unsigned int> entityIds = {1}; // 假设立方体ID为1
        std::vector<double> origin = {0.0, 0.0, 0.0};
        
        if (api->scale(entityIds, origin, 2.0))
        {
            std::cout << "立方体缩放成功！" << std::endl;
        }
        
        // 显示结果
        api->showGeometry();
    }
    else
    {
        std::cerr << "立方体创建失败: " << api->getLastError() << std::endl;
    }
}

/**
 * @brief 演示支持的文件格式
 */
void demonstrateSupportedFormats()
{
    std::cout << "\n=== 支持的文件格式 ===" << std::endl;
    
    ModelProcessor processor;
    
    auto formats = processor.getSupportedFormats();
    
    std::cout << "模型处理器支持以下文件格式:" << std::endl;
    for (const auto& format : formats)
    {
        std::cout << "  - " << format << std::endl;
    }
}

/**
 * @brief 主函数
 */
int main()
{
    std::cout << "=== ModelProcessor 功能演示程序 ===" << std::endl;
    std::cout << "本程序演示新的模型处理器的各种功能" << std::endl;
    
    try
    {
        // 显示支持的格式
        demonstrateSupportedFormats();
        
        // 基本处理功能
        demonstrateBasicProcessing();
        
        // 自定义选项
        demonstrateCustomOptions();
        
        // 网格处理
        demonstrateMeshProcessing();
        
        // 直接API使用
        demonstrateDirectAPIUsage();
        
        // 批量处理（最后执行，因为可能耗时较长）
        demonstrateBatchProcessing();
        
        std::cout << "\n=== 所有演示完成 ===" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "程序执行过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
