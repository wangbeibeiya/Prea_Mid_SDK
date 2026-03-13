#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "VolumeProcessor.h"
#include "RenderProcessor.h"
#include "MeshVisualizationServer.h"
#include "commonEnumeration.h"
#include "../include/ProjectModelData.h"
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometryData.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cmath>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

extern "C" {
    MeshVisualizationServer* GetServerInstance();
}

VolumeProcessor::VolumeProcessor(const std::string& examplePath, Logger* appLogger)
	: m_geometryAPI(std::make_unique<GeometryAPI>(examplePath))
	, m_appLogger(appLogger)
	, m_processedGeometryCount(0)
	, m_processedMeshCount(0)
	, m_processedPostProcessCount(0)
	, m_successfulRepairCount(0)
	, m_foundVolumeCount(0)
{
	// 设置默认处理选项
	m_defaultOptions.enableQuickRepair = true;
	m_defaultOptions.enableFindVolumes = true;
	m_defaultOptions.enableRendering = false;  // 默认不渲染
	m_defaultOptions.repairTolerance = 1e-3;

	Logger* logger = m_appLogger ? m_appLogger : &m_internalLogger;
	m_geometryProcessor = std::make_unique<GeometryProcessor>(m_geometryAPI.get(), logger);
	m_geometryProcessor->setProgressCallback([this](const std::string& msg) { this->printProgress(msg); });
	
	// 初始化网格可视化服务器（默认端口54321）
	m_visualizationServer = std::make_unique<MeshVisualizationServer>(54321);
}

VolumeProcessor::~VolumeProcessor()
{
	if (!m_appLogger)
		m_internalLogger.closeLogFile();
	// 注意：不在这里停止 Socket 服务器
	// Socket 服务器会在渲染窗口关闭时自动停止（通过窗口关闭事件回调）
	// 如果主程序退出时窗口仍然存在，服务器会继续运行
}

bool VolumeProcessor::initialize(bool enableSocketServer)
{
	printProgress("正在初始化模型处理器...");

	if (!m_geometryAPI->initialize())
	{
		setError("几何API初始化失败: " + m_geometryAPI->getLastError());
		return false;
	}

	// 启动网格可视化服务器（仅用于几何渲染）（仅在需要时启动）
	if (enableSocketServer)
	{
		if (m_visualizationServer && !m_visualizationServer->isRunning())
		{
			if (m_visualizationServer->start())
			{
				printProgress("网格可视化服务器已启动，端口: 54321");
			}
			else
			{
				printProgress("警告: 网格可视化服务器启动失败");
			}
		}
	}
	else
	{
		printProgress("Socket服务器已禁用（JSON模式）");
	}

	printProgress("模型处理器初始化成功");
	return true;
}

bool VolumeProcessor::importGeometryModel(const std::string& filePath,
	int importMode,
	ProjectModelData* modelData)
{
	if (!validateFilePath(filePath))
	{
		setError("无效的文件路径: " + filePath);
		return false;
	}

	// 使用 appLogger 时不再单独初始化日志（已在程序启动时创建）
	if (!m_appLogger && modelData)
	{
		std::string workingDir = modelData->getWorkingDirectory();
		std::string projectName = modelData->getProjectName();
		if (!workingDir.empty() && !projectName.empty())
		{
			std::filesystem::path logDirPath(workingDir);
			logDirPath /= projectName;
			try { std::filesystem::create_directories(logDirPath); } catch (...) {}
			std::filesystem::path logFilePath = logDirPath / "MappingGeometry.Log";
			if (m_internalLogger.initializeLogFile(logFilePath.string()))
			{
				m_internalLogger.logOutput("========================================");
				m_internalLogger.logOutput("几何模型导入日志");
				m_internalLogger.logOutput("开始时间: " + m_internalLogger.getCurrentTimeString());
				m_internalLogger.logOutput("模型文件: " + filePath);
				m_internalLogger.logOutput("========================================\n");
			}
		}
	}
	else if (m_appLogger)
	{
		m_appLogger->logOutput("========================================");
		m_appLogger->logOutput("几何模型导入 - " + filePath);
		m_appLogger->logOutput("========================================\n");
	}

	printProgress("正在导入几何文件: " + filePath);
	m_currentImportFilePath = filePath;

		if (!m_geometryAPI->importGeometry(filePath, importMode))
		{
			setError("导入几何文件失败: " + m_geometryAPI->getLastError());
			updateStats("geometry", false);
			return false;
		}

	printProgress("几何文件导入成功，顶层数据已就绪");
	return true;
}

bool VolumeProcessor::executeGeometryProcessing(const ProcessOptions& options)
{
	bool allSuccess = true;

	if (options.enableQuickRepair)
	{
		printProgress("正在执行快速修复操作...");
		if (m_geometryAPI->quickRepair(1e-5))
		{
			printProgress("快速修复操作成功完成！");
			m_successfulRepairCount++;
			}
			else
			{
			printProgress("快速修复操作失败: " + m_geometryAPI->getLastError());
			allSuccess = false;
		}
	}

	if (options.enableFindVolumes)
	{
		printProgress("正在查找体对象...");
		int volumeCount = m_geometryAPI->findVolumes();
		if (volumeCount >= 0)
		{
			printProgress("查找体对象完成！找到 " + std::to_string(volumeCount) + " 个体");
			m_foundVolumeCount += volumeCount;
		}
		else
		{
			printProgress("查找体对象失败: " + m_geometryAPI->getLastError());
			allSuccess = false;
		}
	}

	return allSuccess;
}

bool VolumeProcessor::executeGeometryMatching(ProjectModelData* modelData)
{
	if (!m_geometryProcessor)
	{
		setError("几何处理器未初始化");
		return false;
	}

	// 体已在 executeGeometryProcessing 中通过 findVolumes 找到
	int volumeCount = m_geometryAPI->findVolumes();
	if (volumeCount <= 0)
	{
		printProgress("无体对象可匹配");
		return true;
	}

	std::unordered_map<std::string, std::vector<std::string>> volumeFaceGroupsMap;
	m_lastUnmatchedVolumeNames.clear();
	if (!m_geometryProcessor->analyzeVolumes(modelData, m_defaultOptions.repairTolerance, &m_geometryModel, &volumeFaceGroupsMap, &m_lastUnmatchedVolumeNames))
	{
		setError("几何识别匹配失败");
		return false;
	}

	if (m_outputGroupJson && modelData && !volumeFaceGroupsMap.empty())
	{
		try
		{
			std::string workingDir = modelData->getWorkingDirectory();
			std::string projectName = modelData->getProjectName();
			if (!workingDir.empty() && !projectName.empty())
			{
				std::filesystem::path groupJsonDir(workingDir);
				groupJsonDir /= projectName;
				std::filesystem::create_directories(groupJsonDir);
				std::filesystem::path groupJsonPath = groupJsonDir / "group.json";

				json groupJson;
				json geometryObj;
				json volumesObj;
				for (const auto& pair : volumeFaceGroupsMap)
				{
					json faceGroupsArray = json::array();
					for (const auto& faceGroupName : pair.second)
						faceGroupsArray.push_back(faceGroupName);
					volumesObj[pair.first] = faceGroupsArray;
				}
				geometryObj["Volumes"] = volumesObj;
				groupJson["Geometry"] = geometryObj;

				std::ofstream outFile(groupJsonPath);
				if (outFile.is_open())
				{
					outFile << std::setw(4) << groupJson << std::endl;
					outFile.close();
					(m_appLogger ? m_appLogger : &m_internalLogger)->logOutputLine("✓ 已输出group.json文件: " + groupJsonPath.string());
					printProgress("已输出group.json文件: " + groupJsonPath.string());
				}
			}
		}
		catch (const std::exception& e)
		{
			(m_appLogger ? m_appLogger : &m_internalLogger)->logOutputLine("✗ 输出group.json文件时发生异常: " + std::string(e.what()));
		}
	}

	printProgress("几何识别匹配完成");
	return true;
}

bool VolumeProcessor::refreshGeometryData()
{
	// 几何识别匹配创建新 group 后，需重新调用 getAllData 刷新顶层数据
	PREPRO_BASE_NAMESPACE::PFData data;
	if (!m_geometryAPI->getAllData(data))
	{
		setError("刷新顶层数据失败: " + m_geometryAPI->getLastError());
		return false;
	}
	printProgress("顶层数据已刷新");
	return true;
}

bool VolumeProcessor::saveGeometryPpcf(ProjectModelData* modelData) const
{
	if (!modelData)
		return false;
	std::string workingDir = modelData->getWorkingDirectory();
	std::string projectName = modelData->getProjectName();
	if (workingDir.empty() || projectName.empty())
		return false;

	std::filesystem::path saveDirPath(workingDir);
	saveDirPath /= projectName;
	std::filesystem::create_directories(saveDirPath);
	std::filesystem::path saveFilePath = saveDirPath / (projectName + ".ppcf");

	return m_geometryAPI->saveDocument(saveFilePath.string());
}

bool VolumeProcessor::saveGeometryPpcfToPath(const std::string& ppcfPath)
{
	if (ppcfPath.empty())
	{
		setError("ppcf 路径为空");
		return false;
	}
	return m_geometryAPI->saveDocument(ppcfPath);
}

bool VolumeProcessor::openPpcfAndRunMatching(const std::string& ppcfPath, ProjectModelData* modelData)
{
	if (ppcfPath.empty())
	{
		setError("ppcf 路径为空");
		return false;
	}
	if (!modelData)
	{
		setError("modelData 为空");
		return false;
	}
	printProgress("正在打开 ppcf 并执行几何识别匹配: " + ppcfPath);
	if (!m_geometryAPI->openDocument(ppcfPath))
	{
		setError("打开 ppcf 失败: " + m_geometryAPI->getLastError());
		return false;
	}
	ProcessOptions options;
	options.enableQuickRepair = false;
	options.enableFindVolumes = false;  // ppcf 已包含体结构，无需重复查找
	options.enableRendering = false;
	if (!executeGeometryProcessing(options))
		printProgress("几何处理过程中出现问题，继续执行匹配");
	if (!executeGeometryMatching(modelData))
	{
		setError("几何识别匹配失败");
		return false;
	}
	printProgress("几何识别匹配完成，VolumeRenameMap 已填充");
	return true;
}

bool VolumeProcessor::processGeometryModel(const std::string& filePath,
	int importMode,
	const ProcessOptions& options,
	ProjectModelData* modelData)
{
	if (!validateFilePath(filePath))
	{
		setError("无效的文件路径: " + filePath);
		return false;
	}

	auto start = std::chrono::high_resolution_clock::now();

	try
	{
		// 1. 导入几何模型
		if (!importGeometryModel(filePath, importMode, modelData))
			return false;

		// 2. 几何处理（quickRepair + findVolumes）
		if (!executeGeometryProcessing(options))
			printProgress("几何处理过程中出现问题，但文件已成功导入");

		// 3. 几何识别匹配（analyzeVolumes）
		if (modelData && !executeGeometryMatching(modelData))
		{
			printProgress("几何识别匹配失败");
			return false;
		}

		// 4. 保存几何 ppcf
		if (modelData)
		{
			printProgress("正在保存项目文件...");
			if (saveGeometryPpcf(modelData))
				printProgress("项目文件已成功保存");
			else
					printProgress("保存项目文件失败: " + m_geometryAPI->getLastError());
		}
		else if (!m_currentImportFilePath.empty())
		{
			// 如果没有modelData，使用导入文件路径生成保存路径
			try
			{
				// 从导入文件路径获取文件夹路径和文件名
				std::filesystem::path importPath(m_currentImportFilePath);
				std::filesystem::path folderPath = importPath.parent_path();
				std::string fileName = importPath.stem().string(); // 不带扩展名的文件名

				// 生成保存路径：导入文件所在文件夹/文件名_processed.ppcf
				std::filesystem::path savePath = folderPath / (fileName + "_processed.ppcf");
				std::string savePathStr = savePath.string();

				printProgress("正在保存项目文件: " + savePathStr);
				if (m_geometryAPI->saveDocument(savePathStr))
				{
					printProgress("项目文件已成功保存到: " + savePathStr);
				}
				else
				{
					printProgress("保存项目文件失败: " + m_geometryAPI->getLastError());
				}
			}
			catch (const std::exception& e)
			{
				printProgress("保存项目文件时发生异常: " + std::string(e.what()));
			}
		}

		// 4. 渲染显示（如果启用，仅几何）
		if (options.enableRendering)
		{
			if (!executeRendering("geometry"))
			{
				printProgress("渲染显示失败，但不影响整体处理结果");
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> totalElapsed = end - start;

		printProgress("几何模型处理完成！总耗时: " + std::to_string(totalElapsed.count()) + " 秒");
		updateStats("geometry", true, m_foundVolumeCount);

		// 关闭日志文件
		Logger* log = m_appLogger ? m_appLogger : &m_internalLogger;
		if (log->isLogFileOpen())
		{
			log->logOutput("\n========================================");
			log->logOutput("几何模型处理完成");
			log->logOutput("结束时间: " + log->getCurrentTimeString());
			log->logOutput("总耗时: " + std::to_string(totalElapsed.count()) + " 秒");
			log->logOutput("========================================\n");
			if (!m_appLogger) m_internalLogger.closeLogFile();
		}

		return true;
	}
	catch (const std::exception& e)
	{
		setError("处理几何模型时发生异常: " + std::string(e.what()));
		updateStats("geometry", false);

		// 关闭日志文件
		Logger* log = m_appLogger ? m_appLogger : &m_internalLogger;
		if (log->isLogFileOpen())
		{
			log->logOutput("\n========================================");
			log->logOutput("几何模型处理失败");
			log->logOutput("错误信息: " + std::string(e.what()));
			log->logOutput("========================================\n");
			if (!m_appLogger) m_internalLogger.closeLogFile();
		}

		return false;
	}
}

bool VolumeProcessor::processGeometryMatchingFromJson(const std::string& jsonPath)
{
	if (!validateFilePath(jsonPath))
	{
		setError("无效的 JSON 路径: " + jsonPath);
		return false;
	}

	ProjectModelData model;
	if (!model.loadFromJsonFile(jsonPath))
	{
		setError("加载 JSON 失败: " + jsonPath);
		return false;
	}

	std::string workingDir = model.getWorkingDirectory();
	std::string projectName = model.getProjectName();
	if (workingDir.empty() || projectName.empty())
	{
		setError("JSON 中缺少 WorkingDirectory 或 ProjectName");
		return false;
	}

	std::filesystem::path modelPath(workingDir);
	modelPath /= projectName;
	modelPath /= projectName + ".stp";
	if (!std::filesystem::exists(modelPath))
	{
		setError("模型文件不存在: " + modelPath.string());
		return false;
	}

	ProcessOptions options;
	options.enableQuickRepair = true;
	options.enableFindVolumes = true;
	options.enableRendering = false;

	return processGeometryModel(modelPath.string(), 1, options, &model);
}

bool VolumeProcessor::processPostProcessResult(const std::string& filePath)
{
	if (!validateFilePath(filePath))
	{
		setError("无效的文件路径: " + filePath);
		return false;
	}

	printProgress("开始处理后处理结果: " + filePath);

	auto start = std::chrono::high_resolution_clock::now();

	try
	{
		// 导入后处理结果
		printProgress("正在导入后处理结果...");
		if (!m_geometryAPI->importPostProcessResult(filePath))
		{
			setError("导入后处理结果失败: " + m_geometryAPI->getLastError());
			updateStats("postprocess", false);
			return false;
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;

		printProgress("后处理结果导入成功！耗时: " + std::to_string(elapsed.count()) + " 秒");
		printProgress("后处理结果已加载，可以使用显示功能查看结果");

		updateStats("postprocess", true);
		return true;
	}
	catch (const std::exception& e)
	{
		setError("处理后处理结果时发生异常: " + std::string(e.what()));
		updateStats("postprocess", false);
		return false;
	}
}

int VolumeProcessor::batchProcessGeometry(const std::vector<std::string>& filePaths,
	int importMode,
	const ProcessOptions& options)
{
	if (filePaths.empty())
	{
		setError("文件路径列表为空");
		return 0;
	}

	printProgress("开始批量处理: " + std::to_string(filePaths.size()) + " 个几何文件");

	int successCount = 0;
	int totalFiles = static_cast<int>(filePaths.size());

	for (int i = 0; i < totalFiles; ++i)
	{
		const std::string& filePath = filePaths[i];

		printProgress("处理进度: " + std::to_string(i + 1) + "/" + std::to_string(totalFiles));

		// 对于批量处理，除了第一个文件，其他文件都使用追加模式
		int currentImportMode = (i == 0) ? importMode : 3; // 3 = 追加模式

		if (processGeometryModel(filePath, currentImportMode, options))
		{
			successCount++;
			printProgress("文件处理成功: " + filePath);
		}
		else
		{
			printProgress("文件处理失败: " + filePath + " (" + getLastError() + ")");
		}
	}

	printProgress("批量处理完成! 成功: " + std::to_string(successCount) +
		"/" + std::to_string(totalFiles));

	return successCount;
}

void VolumeProcessor::setDefaultProcessOptions(const ProcessOptions& options)
{
	m_defaultOptions = options;
}

VolumeProcessor::ProcessOptions VolumeProcessor::getCurrentProcessOptions() const
{
	return m_defaultOptions;
}

std::vector<std::string> VolumeProcessor::getSupportedFormats() const
{
	std::vector<std::string> formats;

	// 几何格式
	formats.push_back("STL - Stereolithography (.stl)");
	formats.push_back("STEP - Standard for Exchange of Product Data (.stp, .step)");
	formats.push_back("IGES - Initial Graphics Exchange Specification (.igs, .iges)");
	formats.push_back("PPCF - Pera CFD Format (.ppcf)");
	formats.push_back("X_T - Parasolid (.x_t)");

	// 网格格式
	formats.push_back("CGNS - CFD General Notation System (.cgns)");
	formats.push_back("FLUENT - ANSYS Fluent Mesh (.msh)");
	formats.push_back("VTK - Visualization Toolkit (.vtk)");

	// 后处理格�?	formats.push_back("Result Files - Post-processing Results (.res, .dat)");

	return formats;
}

bool VolumeProcessor::showCurrentModel(bool showGroups)
{
	try
	{
		printProgress("正在显示当前模型...");

		if (!m_geometryAPI->showGeometry(showGroups))
		{
			setError("显示模型失败: " + m_geometryAPI->getLastError());
			return false;
		}

		printProgress("模型显示完成");
		return true;
	}
	catch (const std::exception& e)
	{
		setError("显示模型时发生异常: " + std::string(e.what()));
		return false;
	}
}

GeometryAPI* VolumeProcessor::getGeometryAPI()
{
	return m_geometryAPI.get();
}

std::string VolumeProcessor::getLastError() const
{
	return m_lastError;
}

std::string VolumeProcessor::getProcessingStats() const
{
	std::string stats;
	stats += "=== 模型处理统计信息 ===\n";
	stats += "已处理几何文�? " + std::to_string(m_processedGeometryCount) + "\n";
	stats += "已处理网格文�? " + std::to_string(m_processedMeshCount) + "\n";
	stats += "已处理后处理文件: " + std::to_string(m_processedPostProcessCount) + "\n";
	stats += "成功修复次数: " + std::to_string(m_successfulRepairCount) + "\n";
	stats += "找到体积总数: " + std::to_string(m_foundVolumeCount) + "\n";
	stats += "总处理文件数: " + std::to_string(m_processedGeometryCount + m_processedMeshCount + m_processedPostProcessCount);

	return stats;
}

void VolumeProcessor::reset()
{
	m_processedGeometryCount = 0;
	m_processedMeshCount = 0;
	m_processedPostProcessCount = 0;
	m_successfulRepairCount = 0;
	m_foundVolumeCount = 0;
	m_lastError.clear();

	printProgress("处理器状态已重置");
}

// ============== 私有方法实现 ==============

void VolumeProcessor::setError(const std::string& error)
{
	m_lastError = error;
	std::cerr << "VolumeProcessor错误: " << error << std::endl;
}


bool VolumeProcessor::executeRendering(const std::string& visualizationType)
{
	try
	{
		printProgress("正在渲染显示几何...");

		// VolumeProcessor 仅支持几何渲染
		VisualizationType vizType = VisualizationType::EGeomerty;

		PREPRO_BASE_NAMESPACE::PFData data;
			if (!m_geometryAPI->getAllData(data))
			{
			setError("获取几何数据失败: " + m_geometryAPI->getLastError());
				return false;
		}
		
		if (m_visualizationServer)
		{
			m_visualizationServer->setVisualizationType(static_cast<int>(vizType));
		}
		
			MeshVisualizationServer* runningServer = GetServerInstance();
			if (runningServer && runningServer->isRunning())
			{
				printProgress("网格可视化服务器运行中，端口: " + std::to_string(runningServer->getPort()));
		}
		
		RenderProcessor::show(data, vizType);
		printProgress("几何渲染显示完成");
		return true;
	}
	catch (const std::exception& e)
	{
		setError("渲染显示时发生异常: " + std::string(e.what()));
		return false;
	}
}

void VolumeProcessor::updateStats(const std::string& type, bool success, int volumeCount)
{
	if (!success) return;

	if (type == "geometry")
	{
		m_processedGeometryCount++;
		if (volumeCount > 0)
		{
			m_foundVolumeCount += volumeCount;
		}
	}
	else if (type == "mesh")
	{
		m_processedMeshCount++;
	}
	else if (type == "postprocess")
	{
		m_processedPostProcessCount++;
	}
}

void VolumeProcessor::printProgress(const std::string& message) const
{
	std::string fullMessage = "[VolumeProcessor] " + message;
	(m_appLogger ? m_appLogger : &m_internalLogger)->logOutputLine(fullMessage);
}


bool VolumeProcessor::validateFilePath(const std::string& filePath) const
{
	if (filePath.empty())
	{
		return false;
	}

	try
	{
		// 检查文件是否存在（如果使用C++17）
#ifdef __cpp_lib_filesystem
		return std::filesystem::exists(filePath);
#else
		// 简单检查：文件路径不为空且包含扩展名
		return filePath.find('.') != std::string::npos;
#endif
	}
	catch (...)
	{
		return false;
	}
}

// 注意：几何处理相关的方法已迁移到 GeometryProcessor 类
// 以下方法已删除，请使用 GeometryProcessor 类中的对应方法
