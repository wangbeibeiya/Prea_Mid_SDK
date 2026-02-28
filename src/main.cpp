#include "ModelProcessor.h"
#include "MeshVisualizationServer.h"
#include "../include/ProjectModelData.h"
#include "../include/Logger.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

// 前向声明：设置全局服务器实例指针
extern "C" {
    void SetServerInstance(MeshVisualizationServer*);
    MeshVisualizationServer* GetServerInstance();
}

int main(int argc, char* argv[]) {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << "=== Pera CFD SDK 模型处理程序 ===" << std::endl;
    
    // 解析命令行参数
    enum class Mode { None, Json, PPCF };
    Mode mode = Mode::None;
    std::string filePath;
    std::string outputPath;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--json") {
            mode = Mode::Json;
            // 下一个参数应该是文件路径
            if (i + 1 < argc) {
                filePath = argv[++i];
            } else {
                std::cerr << "错误: --json 需要指定文件路径" << std::endl;
                return 1;
            }
        } else if (arg == "--ppcf") {
            mode = Mode::PPCF;
            // 下一个参数应该是文件路径
            if (i + 1 < argc) {
                filePath = argv[++i];
            } else {
                std::cerr << "错误: --ppcf 需要指定文件路径" << std::endl;
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "\n用法: MappingGeometry.exe [选项]\n" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --json <路径>     执行几何匹配和网格划分（不渲染，不启动Socket）" << std::endl;
            std::cout << "  --ppcf <路径>     执行网格渲染和Socket启动（不进行几何匹配和网格划分）" << std::endl;
            std::cout << "  --help, -h        显示此帮助信息\n" << std::endl;
            std::cout << "示例:" << std::endl;
            std::cout << "  MappingGeometry.exe --json model.json" << std::endl;
            std::cout << "  MappingGeometry.exe --ppcf model.ppcf" << std::endl;
            return 0;
        } else if (arg[0] != '-') {
            // 如果没有指定模式，可能是旧格式的参数
            if (mode == Mode::None) {
                std::cerr << "错误: 请使用 --json 或 --ppcf 指定处理模式" << std::endl;
                std::cerr << "使用 --help 查看帮助信息" << std::endl;
                return 1;
            }
        }
    }
    
    // 检查是否指定了模式
    if (mode == Mode::None) {
        std::cerr << "错误: 请使用 --json 或 --ppcf 指定处理模式" << std::endl;
        std::cerr << "使用 --help 查看帮助信息" << std::endl;
        return 1;
    }
    
    // 根据模式执行不同的处理流程
    if (mode == Mode::PPCF) {
        // PPCF模式：只执行网格渲染和Socket启动
        std::cout << "\n=== PPCF模式：网格渲染和Socket启动 ===" << std::endl;
        std::cout << "文件路径: " << filePath << std::endl;
        
        try {
            // 在程序启动时立即启动Socket服务器（使用静态变量保持存活）
            std::cout << "\n正在启动Socket服务器..." << std::endl;
            static MeshVisualizationServer socketServer(54321);  // 静态变量，在程序结束前不会销毁
            if (!socketServer.start()) {
                std::cerr << "警告: Socket服务器启动失败，但将继续执行..." << std::endl;
            } else {
                std::cout << "Socket服务器已启动，端口: 54321" << std::endl;
            }
            
            // 创建模型处理器（Socket服务器已在外部启动）
            // 注意：ModelProcessor构造函数会创建新的服务器实例并覆盖全局实例指针
            // 但因为我们传入initialize(false)，它不会启动服务器
            ModelProcessor processor;
            
            // ModelProcessor创建后，全局实例指针被覆盖为processor中的未启动服务器
            // 我们需要重新设置全局实例指针指向已启动的socketServer
            SetServerInstance(&socketServer);
            std::cout << "已设置全局服务器实例指针指向已启动的Socket服务器" << std::endl;
            
            // 初始化SDK（不启动Socket服务器，因为已经在外部启动）
            if (!processor.initialize(false)) {  // false表示不启动Socket服务器
                std::cerr << "SDK初始化失败！" << std::endl;
                return 1;
            }
            
            // 导入PPCF文件并渲染网格
            // 注意：processMeshModel会调用RenderProcessor::show，这会阻塞在主线程直到窗口关闭
            if (processor.processMeshModel(filePath, true)) {  // true表示启用渲染
                std::cout << "\nPPCF文件导入并渲染成功！" << std::endl;
                std::cout << "Socket服务器运行中，端口: 54321" << std::endl;
                std::cout << "渲染窗口已关闭，程序退出" << std::endl;
                return 0;
            } else {
                std::cerr << "\nPPCF文件导入失败！" << std::endl;
                std::cerr << "错误详情: " << processor.getLastError() << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "\n导入PPCF文件时发生错误: " << e.what() << std::endl;
            return 1;
        }
    } else if (mode == Mode::Json) {
        // JSON模式：只执行几何匹配和网格划分
        std::cout << "\n=== JSON模式：几何匹配和网格划分 ===" << std::endl;
        std::string jsonFilePath = filePath;
    std::cout << "\n从 JSON 文件加载数据: " << jsonFilePath << std::endl;
    
    // 创建Logger实例用于记录JSON加载错误
    Logger logger;
    std::string workingDir;
    std::string projectName;
    
    // 尝试从JSON文件中读取WorkingDirectory和ProjectName（即使解析失败）
    try {
        std::ifstream file(jsonFilePath);
        if (file.is_open()) {
            json j;
            file >> j;
            
            // 尝试读取WorkingDirectory和ProjectName
            if (j.contains("WorkingDirectory") && !j["WorkingDirectory"].is_null()) {
                workingDir = j["WorkingDirectory"].get<std::string>();
            }
            if (j.contains("ProjectName") && !j["ProjectName"].is_null()) {
                projectName = j["ProjectName"].get<std::string>();
            }
            file.close();
        }
    } catch (...) {
        // 如果读取失败，忽略，使用空值
    }
    
    // 如果成功读取到工作目录和项目名称，初始化日志文件
    if (!workingDir.empty() && !projectName.empty()) {
        std::filesystem::path logDirPath(workingDir);
        logDirPath /= projectName;
        
        try {
            std::filesystem::create_directories(logDirPath);
        } catch (...) {
            // 如果创建目录失败，继续尝试创建日志文件
        }
        
        std::filesystem::path logFilePath = logDirPath / "MappingGeometry.Log";
        if (logger.initializeLogFile(logFilePath.string())) {
            logger.logOutput("========================================");
            logger.logOutput("程序启动日志");
            logger.logOutput("开始时间: " + logger.getCurrentTimeString());
            logger.logOutput("JSON文件: " + jsonFilePath);
            logger.logOutput("工作目录: " + workingDir);
            logger.logOutput("项目名称: " + projectName);
            logger.logOutput("========================================\n");
        }
    }
    
    ProjectModelData model;
    std::string jsonLoadError;
    bool jsonLoadSuccess = false;
    
    // 先尝试直接解析JSON文件以获取详细的错误信息
    try {
        std::ifstream file(jsonFilePath);
        if (!file.is_open()) {
            jsonLoadError = "无法打开JSON文件: " + jsonFilePath;
        } else {
            try {
                json j;
                file >> j;  // 尝试解析JSON
                file.close();
                
                // JSON解析成功，尝试加载到ProjectModelData
                try {
                    jsonLoadSuccess = model.loadFromJsonFile(jsonFilePath);
                    if (!jsonLoadSuccess) {
                        jsonLoadError = "JSON文件解析成功，但加载到ProjectModelData失败";
                    }
                } catch (const json::type_error& e) {
                    // 类型错误，尝试从错误信息中提取字段名
                    std::string errorMsg = e.what();
                    std::string detailedError = "加载到ProjectModelData时JSON类型错误: " + errorMsg;
                    
                    // 尝试从错误信息中提取字段信息
                    // nlohmann/json的错误信息格式通常是: [json.exception.type_error.302] type must be string, but is null
                    // 如果我们在fromJson中添加了字段名，错误信息会包含字段名
                    if (errorMsg.find("字段") != std::string::npos) {
                        detailedError += "\n  提示: 错误信息中已包含字段名";
                    } else {
                        // 尝试从JSON对象中找出null值的字段
                        try {
                            std::ifstream checkFile(jsonFilePath);
                            json checkJ;
                            checkFile >> checkJ;
                            checkFile.close();
                            
                            // 检查常见字段是否为null
                            std::vector<std::string> nullFields;
                            if (checkJ.contains("WorkingDirectory") && checkJ["WorkingDirectory"].is_null()) {
                                nullFields.push_back("WorkingDirectory");
                            }
                            if (checkJ.contains("ProjectName") && checkJ["ProjectName"].is_null()) {
                                nullFields.push_back("ProjectName");
                            }
                            if (checkJ.contains("Name") && checkJ["Name"].is_null()) {
                                nullFields.push_back("Name");
                            }
                            if (checkJ.contains("Description") && checkJ["Description"].is_null()) {
                                nullFields.push_back("Description");
                            }
                            
                            if (!nullFields.empty()) {
                                detailedError += "\n  检测到以下字段为null值: ";
                                for (size_t i = 0; i < nullFields.size(); ++i) {
                                    if (i > 0) detailedError += ", ";
                                    detailedError += nullFields[i];
                                }
                            }
                        } catch (...) {
                            // 忽略检查错误
                        }
                    }
                    
                    jsonLoadError = detailedError;
                } catch (const json::exception& e) {
                    jsonLoadError = std::string("加载到ProjectModelData时JSON解析错误: ") + e.what();
                } catch (const std::exception& e) {
                    jsonLoadError = std::string("加载到ProjectModelData时发生错误: ") + e.what();
                }
            } catch (const json::exception& e) {
                jsonLoadError = std::string("JSON解析错误: ") + e.what();
            } catch (const std::exception& e) {
                jsonLoadError = std::string("读取JSON文件失败: ") + e.what();
            }
        }
    } catch (const std::exception& e) {
        jsonLoadError = std::string("打开JSON文件时发生错误: ") + e.what();
    } catch (...) {
        jsonLoadError = "JSON文件加载失败（未知错误）";
    }
    
    if (jsonLoadSuccess) {
        std::cout << "JSON 文件加载成功！" << std::endl;
        model.printSummary();
        
        // 显示加载的数据统计
        std::cout << "\n数据统计:" << std::endl;
        std::cout << "  ModelTree 项数: " << model.getModelTreeCount() << std::endl;
        std::cout << "  SetList 集合数: " << model.getSetListCount() << std::endl;
        std::cout << "  AllSolidList 实体数: " << model.getAllSolidListCount() << std::endl;
        
        // 从 JSON 中获取工作路径和项目名称
        std::string workingDir = model.getWorkingDirectory();
        std::string projectName = model.getProjectName();
        
        // 构建模型路径: 工作路径 + 项目名称 + 项目名称.stp
        std::string modelPath;
        if (!workingDir.empty() && !projectName.empty()) {
            modelPath = workingDir;
            // 确保路径末尾有路径分隔符
            if (modelPath.back() != '\\' && modelPath.back() != '/') {
                modelPath += "\\";
            }
            modelPath += projectName + "\\" + projectName + ".stp";
        }
        
        if (!modelPath.empty()) {
            std::cout << "\n准备导入几何模型..." << std::endl;
            if (!workingDir.empty()) {
                std::cout << "  工作目录: " << workingDir << std::endl;
            }
            if (!projectName.empty()) {
                std::cout << "  项目名称: " << projectName << std::endl;
            }
            std::cout << "  模型路径: " << modelPath << std::endl;
            std::cout << "  (从 WorkingDirectory + ProjectName + ProjectName.stp 构建)" << std::endl;
            
            try {
                // 创建模型导入器（不启动Socket服务器）
                ModelProcessor processor;
                
                // 初始化SDK（不启动Socket服务器）
                if (!processor.initialize(false)) {  // false表示不启动Socket服务器
                    std::cerr << "SDK初始化失败！" << std::endl;
                    return 1;
                }
                
                // 设置导入选项
                ModelProcessor::ProcessOptions options;
                options.enableQuickRepair = true;
                options.enableFindVolumes = true;
                options.enableRendering = false;  // JSON模式不渲染
                
                // 使用第一次导入模式（值为1）
                int importMode = 1;
                
                // 导入几何模型（传入 modelData 用于匹配和重命名）
                if (processor.processGeometryModel(modelPath, importMode, options, &model)) {
                    std::cout << "\n几何模型导入成功！" << std::endl;
                } else {
                    std::cerr << "\n几何模型导入失败！" << std::endl;
                    std::cerr << "错误详情: " << processor.getLastError() << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "\n导入模型时发生错误: " << e.what() << std::endl;
                return 1;
            }
        } else {
            std::cout << "\n警告: 无法从 WorkingDirectory 和 ProjectName 构建模型路径，跳过模型导入" << std::endl;
            if (workingDir.empty()) {
                std::cout << "  WorkingDirectory 为空" << std::endl;
            }
            if (projectName.empty()) {
                std::cout << "  ProjectName 为空" << std::endl;
            }
        }
        
        // 如果指定了输出文件，保存处理后的数据
        if (!outputPath.empty()) {
            if (model.saveToJsonFile(outputPath)) {
                std::cout << "\n数据已保存到: " << outputPath << std::endl;
            } else {
                std::cerr << "保存文件失败！" << std::endl;
            }
        }
        
        return 0;
    } else {
        // JSON加载失败，记录到日志
        std::cerr << "JSON 文件加载失败！" << std::endl;
        std::cerr << "文件路径: " << jsonFilePath << std::endl;
        
        if (logger.isLogFileOpen()) {
            logger.logOutput("========================================");
            logger.logOutput("错误: JSON文件加载失败");
            logger.logOutput("========================================");
            logger.logOutput("文件路径: " + jsonFilePath);
            logger.logOutput("错误信息: " + jsonLoadError);
            logger.logOutput("时间: " + logger.getCurrentTimeString());
            logger.logOutput("========================================\n");
        } else {
            // 如果日志文件未打开，尝试使用默认路径创建日志文件
            try {
                std::filesystem::path jsonPath(jsonFilePath);
                std::filesystem::path jsonDir = jsonPath.parent_path();
                if (std::filesystem::exists(jsonDir)) {
                    std::filesystem::path defaultLogPath = jsonDir / "MappingGeometry.Log";
                    if (logger.initializeLogFile(defaultLogPath.string())) {
                        logger.logOutput("========================================");
                        logger.logOutput("错误: JSON文件加载失败");
                        logger.logOutput("========================================");
                        logger.logOutput("文件路径: " + jsonFilePath);
                        logger.logOutput("错误信息: " + jsonLoadError);
                        logger.logOutput("时间: " + logger.getCurrentTimeString());
                        logger.logOutput("========================================\n");
                    }
                }
            } catch (...) {
                // 如果创建日志文件失败，忽略
            }
        }
        
        return 1;
    }
} 
} 