#include "ModelProcessor.h"
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

int main(int argc, char* argv[]) {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::cout << "=== Pera CFD SDK 模型导入演示程序 ===" << std::endl;
    
    // 获取文件路径
    std::string filePath;
    if (argc > 1) {
        // 从命令行参数获取
        filePath = argv[1];
    } else {
        // 使用默认路径
        //filePath = "F:\\PeraSimTestPro\\f98\\T1230/T1230.json";
        filePath = "F:\\PeraSimTestPro\\f98\\T1230/T1230.ppcf";
        std::cout << "\n未指定文件路径，使用默认路径: " << filePath << std::endl;
        std::cout << "提示: 您也可以通过命令行参数指定文件路径（支持JSON和PPCF格式）" << std::endl;
    }
    
    // 检查文件扩展名，判断是否为PPCF文件
    std::filesystem::path pathObj(filePath);
    std::string extension = pathObj.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // 如果是PPCF文件，直接导入并渲染网格，跳过几何匹配和网格划分
    if (extension == ".ppcf") {
        std::cout << "\n检测到PPCF文件，直接导入并渲染网格..." << std::endl;
        std::cout << "文件路径: " << filePath << std::endl;
        
        try {
            // 创建模型处理器
            ModelProcessor processor;
            
            // 初始化SDK
            if (!processor.initialize()) {
                std::cerr << "SDK初始化失败！" << std::endl;
                return 1;
            }
            
            // 直接导入PPCF文件并渲染网格
            if (processor.processMeshModel(filePath, true)) {
                std::cout << "\nPPCF文件导入并渲染成功！" << std::endl;
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
    }
    
    // 如果不是PPCF文件，按原来的流程处理（JSON文件 -> 几何导入 -> 网格生成）
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
                // 创建模型导入器
                ModelProcessor processor;
                
                // 初始化SDK
                if (!processor.initialize()) {
                    std::cerr << "SDK初始化失败！" << std::endl;
                    return 1;
                }
                
                // 设置导入选项
                ModelProcessor::ProcessOptions options;
                options.enableQuickRepair = true;
                options.enableFindVolumes = true;
                options.enableRendering = true;  // 默认渲染网格（如果已生成）
                
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
        if (argc > 2) {
            std::string outputPath = argv[2];
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