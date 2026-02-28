#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "ModelProcessor.h"
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

ModelProcessor::ModelProcessor(const std::string& examplePath)
	: m_geometryAPI(std::make_unique<GeometryAPI>(examplePath))
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

	// 初始化几何处理器
	m_geometryProcessor = std::make_unique<GeometryProcessor>(m_geometryAPI.get(), &m_logger);
	m_geometryProcessor->setProgressCallback([this](const std::string& msg) { this->printProgress(msg); });
	
	// 初始化网格可视化服务器（默认端口54321）
	m_visualizationServer = std::make_unique<MeshVisualizationServer>(54321);
}

ModelProcessor::~ModelProcessor()
{
	m_logger.closeLogFile();
	// 注意：不在这里停止 Socket 服务器
	// Socket 服务器会在渲染窗口关闭时自动停止（通过窗口关闭事件回调）
	// 如果主程序退出时窗口仍然存在，服务器会继续运行
}

bool ModelProcessor::initialize(bool enableSocketServer)
{
	printProgress("正在初始化模型处理器...");

	if (!m_geometryAPI->initialize())
	{
		setError("几何API初始化失败: " + m_geometryAPI->getLastError());
		return false;
	}

	// 初始化网格处理器
	PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = m_geometryAPI->getDocument();
	if (pfDocument)
	{
		m_meshProcessor = std::make_unique<MeshProcessor>(pfDocument);
		if (m_meshProcessor->initialize())
		{
			m_meshProcessor->setProgressCallback([this](const std::string& msg) { this->printProgress(msg); });
		}
		else
		{
			printProgress("警告: 网格处理器初始化失败: " + m_meshProcessor->getLastError());
		}
	}

	// 启动网格可视化服务器（仅在需要时启动）
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

bool ModelProcessor::processGeometryModel(const std::string& filePath,
	int importMode,
	const ProcessOptions& options,
	ProjectModelData* modelData)
{
	if (!validateFilePath(filePath))
	{
		setError("无效的文件路径: " + filePath);
		return false;
	}

	// 如果提供了modelData，初始化日志文件
	if (modelData)
	{
		std::string workingDir = modelData->getWorkingDirectory();
		std::string projectName = modelData->getProjectName();
		if (!workingDir.empty() && !projectName.empty())
		{
			// 构建日志文件路径：工作路径 + 项目名称文件夹 + MappingGeometry.Log
			std::filesystem::path logDirPath(workingDir);
			logDirPath /= projectName;

			// 确保目录存在
			try
			{
				std::filesystem::create_directories(logDirPath);
			}
			catch (const std::exception& e)
			{
				// 如果创建目录失败，继续尝试创建日志文件（可能目录已存在）
			}

			// 构建完整的日志文件路径
			std::filesystem::path logFilePath = logDirPath / "MappingGeometry.Log";

			if (m_logger.initializeLogFile(logFilePath.string()))
			{
				m_logger.logOutput("========================================");
				m_logger.logOutput("几何模型处理日志");
				m_logger.logOutput("开始时间: " + m_logger.getCurrentTimeString());
				m_logger.logOutput("模型文件: " + filePath);
				m_logger.logOutput("工作目录: " + workingDir);
				m_logger.logOutput("项目名称: " + projectName);
				m_logger.logOutput("日志文件: " + logFilePath.string());
				m_logger.logOutput("========================================\n");
			}
		}
	}

	printProgress("开始处理几何模型: " + filePath);

	// 保存当前导入的文件路径
	m_currentImportFilePath = filePath;

	auto start = std::chrono::high_resolution_clock::now();

	try
	{
		// 1. 导入几何文件
		printProgress("正在导入几何文件...");
		if (!m_geometryAPI->importGeometry(filePath, importMode))
		{
			setError("导入几何文件失败: " + m_geometryAPI->getLastError());
			updateStats("geometry", false);
			return false;
		}

		auto importEnd = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> importElapsed = importEnd - start;
		printProgress("几何文件导入成功！耗时: " + std::to_string(importElapsed.count()) + " 秒");

		// 2. 执行几何处理
		if (!executeGeometryProcessing(options, modelData))
		{
			// 处理失败但导入成功，仍然算作部分成功
			printProgress("几何处理过程中出现问题，但文件已成功导入");
		}

		// 3. 设置网格参数并生成网格（如果启用）
		if (m_enableMeshGeneration && m_meshProcessor)
		{
			// 确保MeshProcessor已初始化（在导入几何后，确保能获取到PFMesh）
			if (!m_meshProcessor->initialize())
			{
				printProgress("警告: 网格处理器初始化失败，跳过网格参数设置和网格生成: " + m_meshProcessor->getLastError());
			}
			else
			{
				// 3.1 设置网格参数（从JSON的FluidMeshInfo中读取）
				if (modelData)
				{
					const auto& fluidMeshInfo = modelData->getFluidMeshInfo();

					// 创建网格参数，优先使用JSON中的值，如果没有则使用默认值
					MeshProcessor::MeshParameters meshParams;

					if (fluidMeshInfo.MinMeshSize.has_value())
					{
						// JSON中的值是毫米，需要转换为米（除以1000）
						meshParams.minSize = fluidMeshInfo.MinMeshSize.value() / 1000.0;
					}

					if (fluidMeshInfo.MaxMeshSize.has_value())
					{
						// JSON中的值是毫米，需要转换为米（除以1000）
						meshParams.maxSize = fluidMeshInfo.MaxMeshSize.value() / 1000.0;
					}

					if (fluidMeshInfo.GrowthRate.has_value())
					{
						meshParams.growthRate = fluidMeshInfo.GrowthRate.value();
					}

					if (fluidMeshInfo.NormalAngle.has_value())
					{
						meshParams.curvatureNormalAngle = fluidMeshInfo.NormalAngle.value();
					}

					// 从JSON加载边界层参数（如果存在）
					// 由于ProjectModelData的toJson()不包含BoundaryLayersInfo，我们需要从原始JSON读取
					// 方法：通过modelData的WorkingDirectory和ProjectName构建JSON文件路径
					std::vector<LocalFluidMeshItem> localMeshItems;
					if (modelData && !modelData->getLocalFluidMeshsInfo().empty())
					{
						localMeshItems = modelData->getLocalFluidMeshsInfo();
					}
					try {
						std::string workingDir = modelData->getWorkingDirectory();
						std::string projectName = modelData->getProjectName();

						if (!workingDir.empty() && !projectName.empty())
						{
							// 构建JSON文件路径：工作目录 + 项目名称 + 项目名称.json
							std::filesystem::path jsonFilePath(workingDir);
							jsonFilePath /= projectName;
							jsonFilePath /= projectName + ".json";

							// 尝试读取JSON文件
							if (std::filesystem::exists(jsonFilePath))
							{
								std::ifstream jsonFile(jsonFilePath);
								if (jsonFile.is_open())
								{
									json originalJson;
									jsonFile >> originalJson;
									jsonFile.close();

									// 读取BoundaryConditionInfo，提取非Wall类型的边界名称
									std::set<std::string> excludedBoundaryNames;
									if (originalJson.contains("BoundaryConditionInfo") && originalJson["BoundaryConditionInfo"].is_object())
									{
										const auto& bcInfo = originalJson["BoundaryConditionInfo"];
										if (bcInfo.contains("BoundaryConditions") && bcInfo["BoundaryConditions"].is_array())
										{
											for (const auto& bc : bcInfo["BoundaryConditions"])
											{
												if (bc.is_object())
												{
													std::string bcType;
													std::string bcName;

													if (bc.contains("Type") && !bc["Type"].is_null())
													{
														bcType = bc["Type"].get<std::string>();
													}
													if (bc.contains("Name") && !bc["Name"].is_null())
													{
														bcName = bc["Name"].get<std::string>();
													}

													// 如果边界类型不是Wall或WallModel，则添加到排除列表
													if (!bcType.empty() && !bcName.empty())
													{
														// 检查是否为Wall类型（可能的值：Wall, WallModel等）
														if (bcType != "Wall" && bcType != "WallModel")
														{
															excludedBoundaryNames.insert(bcName);
															printProgress("检测到非Wall类型边界条件: Type=" + bcType + ", Name=" + bcName + " (将跳过边界层设置)");
														}
													}
												}
											}
										}
									}

									// 读取BoundaryLayersInfo
									if (originalJson.contains("BoundaryLayersInfo") && originalJson["BoundaryLayersInfo"].is_object())
									{
										const auto& blInfo = originalJson["BoundaryLayersInfo"];

										MeshProcessor::BoundaryLayerParameters blParams;
										if (blInfo.contains("FirstLayerHeight"))
										{
											blParams.firstLayerHeight = blInfo["FirstLayerHeight"].get<double>();
										}
										if (blInfo.contains("GrowthRate"))
										{
											blParams.growthRate = blInfo["GrowthRate"].get<double>();
										}
										if (blInfo.contains("LayersNumber"))
										{
											blParams.layersNumber = blInfo["LayersNumber"].get<int>();
										}
										if (blInfo.contains("IsBoundaryLayers"))
										{
											blParams.isBoundaryLayers = blInfo["IsBoundaryLayers"].get<bool>();
										}

										std::string fluidZoneSet;
										if (blInfo.contains("FluidZoneSet") && !blInfo["FluidZoneSet"].is_null())
										{
											fluidZoneSet = blInfo["FluidZoneSet"].get<std::string>();
										}

										// 如果边界层已启用且有FluidZoneSet，则添加到meshParams中
										if (blParams.isBoundaryLayers && !fluidZoneSet.empty())
										{
											meshParams.boundaryLayerParams = blParams;
											meshParams.fluidZoneSetName = fluidZoneSet;
											meshParams.excludedBoundaryNames = excludedBoundaryNames;  // 保存排除的边界名称列表
											printProgress("已从JSON加载边界层参数: FirstLayerHeight=" + std::to_string(blParams.firstLayerHeight) +
												", GrowthRate=" + std::to_string(blParams.growthRate) +
												", LayersNumber=" + std::to_string(blParams.layersNumber) +
												", FluidZoneSet=" + fluidZoneSet);

											// 如果有排除的边界名称，记录日志
											if (!excludedBoundaryNames.empty())
											{
												std::string excludedList = "排除的边界名称: ";
												for (const auto& name : excludedBoundaryNames)
												{
													excludedList += name + " ";
												}
												printProgress(excludedList);
											}
										}
									}
									
									// 若 modelData 中无 LocalFluidMeshsInfo，则从 JSON 解析
									if (localMeshItems.empty() &&
										originalJson.contains("LocalFluidMeshsInfo") && originalJson["LocalFluidMeshsInfo"].is_object())
									{
										const auto& lfmInfo = originalJson["LocalFluidMeshsInfo"];
										if (lfmInfo.contains("LocalFluidMeshsInfo") && lfmInfo["LocalFluidMeshsInfo"].is_array())
										{
											for (const auto& itemJson : lfmInfo["LocalFluidMeshsInfo"])
											{
												LocalFluidMeshItem lfmItem;
												lfmItem.fromJson(itemJson);
												localMeshItems.push_back(lfmItem);
											}
										}
									}
								}
							}
						}
					}
					catch (const std::exception& e)
					{
						printProgress("警告: 读取边界层参数时发生错误: " + std::string(e.what()));
					}

					// 设置网格参数（传递modelData以便边界层设置时使用SetList）
					if (m_meshProcessor->setGlobalParameters(meshParams, modelData))
					{
						printProgress("已从JSON加载网格参数: MinSize=" + std::to_string(meshParams.minSize) +
							", MaxSize=" + std::to_string(meshParams.maxSize) +
							", GrowthRate=" + std::to_string(meshParams.growthRate) +
							", NormalAngle=" + std::to_string(meshParams.curvatureNormalAngle));
					}
					else
					{
						printProgress("警告: 设置网格参数失败: " + m_meshProcessor->getLastError());
					}
					
					// 设置局部网格参数（从 LocalFluidMeshsInfo 读取，需在生成网格前设置）
					if (!localMeshItems.empty() && modelData)
					{
						if (m_meshProcessor->setLocalMeshParameters(localMeshItems, modelData))
						{
							printProgress("已设置局部网格参数，共 " + std::to_string(localMeshItems.size()) + " 项");
						}
						else
						{
							printProgress("警告: 设置局部网格参数失败: " + m_meshProcessor->getLastError());
						}
					}
				}

				// 3.2 生成网格
				printProgress("正在生成网格...");
				if (m_meshProcessor->createVolumeMeshByGeometry(true, true))
				{
					// 验证网格数据是否存在
					if (m_meshProcessor->hasMeshData())
					{
						printProgress("网格生成成功，已验证网格数据存在");
					}
					else
					{
						printProgress("警告: 网格生成返回成功，但未检测到网格数据");
					}
				}
				else
				{
					printProgress("网格生成失败: " + m_meshProcessor->getLastError());
				}
			}
		}

		// 4. 保存项目为ppcf文件
		// 注意：如果启用了网格生成，保存前会验证网格数据是否存在
		if (m_enableMeshGeneration && m_meshProcessor)
		{
			if (!m_meshProcessor->hasMeshData())
			{
				printProgress("警告: 保存ppcf文件前未检测到网格数据，保存的文件可能不包含网格");
			}
		}
		if (modelData)
		{
			std::string workingDir = modelData->getWorkingDirectory();
			std::string projectName = modelData->getProjectName();
			if (!workingDir.empty() && !projectName.empty())
			{
				// 构建保存路径：工作路径 + 项目名称文件夹 + 项目名称.ppcf
				std::filesystem::path saveDirPath(workingDir);
				saveDirPath /= projectName;

				// 确保目录存在
				std::filesystem::create_directories(saveDirPath);

				// 构建完整的保存文件路径
				std::filesystem::path saveFilePath = saveDirPath / (projectName + ".ppcf");

				printProgress("正在保存项目文件: " + saveFilePath.string());
				if (m_geometryAPI->saveDocument(saveFilePath.string()))
				{
					printProgress("项目文件已成功保存到: " + saveFilePath.string());
				}
				else
				{
					printProgress("保存项目文件失败: " + m_geometryAPI->getLastError());
				}
			}
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

		// 5. 渲染显示（如果启用）
		// 优先在网格生成完成后只渲染网格；如果未生成网格则渲染几何
		if (options.enableRendering)
		{
			const bool canRenderMesh = (m_meshProcessor && m_meshProcessor->hasMeshData());
			const std::string renderType = canRenderMesh ? "mesh" : "geometry";
			printProgress(std::string("渲染开关已启用，当前将渲染: ") + (canRenderMesh ? "mesh(只显示网格)" : "geometry"));

			if (!executeRendering(renderType))
			{
				printProgress("渲染显示失败，但不影响整体处理结果");
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> totalElapsed = end - start;

		printProgress("几何模型处理完成！总耗时: " + std::to_string(totalElapsed.count()) + " 秒");
		updateStats("geometry", true, m_foundVolumeCount);

		// 关闭日志文件
		if (m_logger.isLogFileOpen())
		{
			m_logger.logOutput("\n========================================");
			m_logger.logOutput("几何模型处理完成");
			m_logger.logOutput("结束时间: " + m_logger.getCurrentTimeString());
			m_logger.logOutput("总耗时: " + std::to_string(totalElapsed.count()) + " 秒");
			m_logger.logOutput("========================================\n");
			m_logger.closeLogFile();
		}

		return true;
	}
	catch (const std::exception& e)
	{
		setError("处理几何模型时发生异常: " + std::string(e.what()));
		updateStats("geometry", false);

		// 关闭日志文件
		if (m_logger.isLogFileOpen())
		{
			m_logger.logOutput("\n========================================");
			m_logger.logOutput("几何模型处理失败");
			m_logger.logOutput("错误信息: " + std::string(e.what()));
			m_logger.logOutput("========================================\n");
			m_logger.closeLogFile();
		}

		return false;
	}
}

bool ModelProcessor::processMeshModel(const std::string& filePath, bool enableRendering)
{
	if (!validateFilePath(filePath))
	{
		setError("无效的文件路径: " + filePath);
		return false;
	}

	printProgress("开始处理网格模型: " + filePath);

	auto start = std::chrono::high_resolution_clock::now();

	try
	{
		// 导入网格文件
		printProgress("正在导入网格文件...");
		if (!m_geometryAPI->importMesh(filePath))
		{
			setError("导入网格文件失败: " + m_geometryAPI->getLastError());
			updateStats("mesh", false);
			return false;
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;

		printProgress("网格文件导入成功！耗时: " + std::to_string(elapsed.count()) + " 秒");

		// 重新获取文档并初始化MeshProcessor（因为importMesh可能创建了新文档）
		PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = m_geometryAPI->getDocument();
		if (pfDocument)
		{
			// 如果MeshProcessor已存在，需要重新创建以使用新的文档
			if (m_meshProcessor)
			{
				m_meshProcessor.reset();
			}
			m_meshProcessor = std::make_unique<MeshProcessor>(pfDocument);
			if (m_meshProcessor->initialize())
			{
				m_meshProcessor->setProgressCallback([this](const std::string& msg) { this->printProgress(msg); });
				printProgress("网格处理器已初始化");
			}
			else
			{
				printProgress("警告: 网格处理器初始化失败: " + m_meshProcessor->getLastError());
			}
		}
		else
		{
			printProgress("警告: 无法获取文档，网格处理器可能无法正常工作");
		}

		// 渲染显示（如果启用）
		if (enableRendering)
		{
			if (!executeRendering("mesh"))
			{
				printProgress("渲染显示失败，但不影响导入结果");
			}
		}

		updateStats("mesh", true);
		return true;
	}
	catch (const std::exception& e)
	{
		setError("处理网格模型时发生异常: " + std::string(e.what()));
		updateStats("mesh", false);
		return false;
	}
}

bool ModelProcessor::processPostProcessResult(const std::string& filePath)
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

int ModelProcessor::batchProcessGeometry(const std::vector<std::string>& filePaths,
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

void ModelProcessor::setDefaultProcessOptions(const ProcessOptions& options)
{
	m_defaultOptions = options;
}

ModelProcessor::ProcessOptions ModelProcessor::getCurrentProcessOptions() const
{
	return m_defaultOptions;
}

std::vector<std::string> ModelProcessor::getSupportedFormats() const
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

bool ModelProcessor::showCurrentModel(bool showGroups)
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

GeometryAPI* ModelProcessor::getGeometryAPI()
{
	return m_geometryAPI.get();
}

std::string ModelProcessor::getLastError() const
{
	return m_lastError;
}

std::string ModelProcessor::getProcessingStats() const
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

void ModelProcessor::reset()
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

void ModelProcessor::setError(const std::string& error)
{
	m_lastError = error;
	std::cerr << "ModelProcessor错误: " << error << std::endl;
}

bool ModelProcessor::executeGeometryProcessing(const ProcessOptions& options, ProjectModelData* modelData)
{
	bool allSuccess = true;

	try
	{
		// 执行快速修复
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

		// 执行查找体积
		if (options.enableFindVolumes)
		{
			printProgress("正在查找体对象...");
			int volumeCount = m_geometryAPI->findVolumes();
			if (volumeCount >= 0)
			{
				printProgress("查找体对象完成！找到 " + std::to_string(volumeCount) + " 个体");
				m_foundVolumeCount += volumeCount;

				// 输出每个体的详细信息
				if (volumeCount > 0)
				{
					// 使用几何处理器进行分析
					if (!m_geometryProcessor)
					{
						setError("几何处理器未初始化");
						allSuccess = false;
					}
					else
					{
						std::unordered_map<std::string, std::vector<std::string>> volumeFaceGroupsMap;
						if (m_geometryProcessor->analyzeVolumes(modelData, options.repairTolerance, &m_geometryModel, &volumeFaceGroupsMap))
						{
							// 输出group.json文件（如果启用）
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
											{
												faceGroupsArray.push_back(faceGroupName);
											}
											volumesObj[pair.first] = faceGroupsArray;
										}

										geometryObj["Volumes"] = volumesObj;
										groupJson["Geometry"] = geometryObj;

										std::ofstream outFile(groupJsonPath);
										if (outFile.is_open())
										{
											outFile << std::setw(4) << groupJson << std::endl;
											outFile.close();
											m_logger.logOutputLine("✓ 已输出group.json文件: " + groupJsonPath.string());
											printProgress("已输出group.json文件: " + groupJsonPath.string());
										}
									}
								}
								catch (const std::exception& e)
								{
									m_logger.logOutputLine("✗ 输出group.json文件时发生异常: " + std::string(e.what()));
								}
							}
						}
						else
						{
							setError("几何处理失败");
							allSuccess = false;
						}
					}
				}
			}
			else
			{
				printProgress("查找体对象失败: " + m_geometryAPI->getLastError());
				allSuccess = false;
			}
		}

		return allSuccess;
	}
	catch (const std::exception& e)
	{
		setError("几何处理过程中发生异常: " + std::string(e.what()));
		return false;
	}
}

bool ModelProcessor::executeRendering(const std::string& visualizationType)
{
	try
	{
		printProgress("正在渲染显示模型...");

		// 根据类型选择可视化方式
		VisualizationType vizType;
		if (visualizationType == "geometry")
		{
			vizType = VisualizationType::EGeomerty;
		}
		else if (visualizationType == "mesh")
		{
			vizType = VisualizationType::EMesh;
		}
		else
		{
			vizType = VisualizationType::EGeomerty; // 默认
		}

		// 获取数据并显示
		PREPRO_BASE_NAMESPACE::PFData data;
		if (vizType == VisualizationType::EMesh)
		{
			// 只显示网格：从PFMesh环境获取数据（避免混入几何数据）
			if (!m_meshProcessor)
			{
				setError("获取网格数据失败: MeshProcessor未初始化");
				return false;
			}
			
			if (!m_meshProcessor->getMeshData(data))
			{
				// 尝试获取更详细的错误信息
				std::string errorMsg = "获取网格数据失败";
				std::string meshError = m_meshProcessor->getLastError();
				if (!meshError.empty())
				{
					errorMsg += ": " + meshError;
				}
				else
				{
					// 检查数据是否为空
					if (data.getGroupSize() == 0 && data.getVolumeSize() == 0)
					{
						errorMsg += ": 网格数据为空（没有组或体积）";
					}
					else
					{
						errorMsg += ": 无法从PFMesh获取数据";
					}
				}
				setError(errorMsg);
				printProgress(errorMsg);
				return false;
			}
			
			// 输出调试信息
			printProgress("成功获取网格数据: 组数=" + std::to_string(data.getGroupSize()) + 
				", 体积数=" + std::to_string(data.getVolumeSize()));
		}
		else
		{
			// 显示几何：从PFGeometry环境获取数据
			if (!m_geometryAPI->getAllData(data))
			{
				setError("获取模型数据失败: " + m_geometryAPI->getLastError());
				return false;
			}
		}
		
		// 设置渲染数据到服务器（供Socket命令使用）
		// 注意：由于PFData不能复制，我们只设置类型，实际数据通过SetRenderSession在RenderProcessor中设置
		if (m_visualizationServer && vizType == VisualizationType::EMesh)
		{
			m_visualizationServer->setVisualizationType(static_cast<int>(vizType));
		}
		
		// 执行渲染
		// 注意：服务器已在程序启动时启动，SetRenderSession 会设置窗口句柄
		// 在 show() 阻塞前检查服务器状态（show 返回时窗口已关闭、服务器已停止）
		if (vizType == VisualizationType::EMesh)
		{
			MeshVisualizationServer* runningServer = GetServerInstance();
			if (runningServer && runningServer->isRunning())
			{
				printProgress("网格可视化服务器运行中，端口: 54321");
			}
			else
			{
				printProgress("警告: 网格可视化服务器未运行");
			}
		}
		
		RenderProcessor::show(data, vizType);
		
		printProgress("模型渲染显示完成");

		return true;
	}
	catch (const std::exception& e)
	{
		setError("渲染显示时发生异常: " + std::string(e.what()));
		return false;
	}
}

void ModelProcessor::updateStats(const std::string& type, bool success, int volumeCount)
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

void ModelProcessor::printProgress(const std::string& message) const
{
	std::string fullMessage = "[ModelProcessor] " + message;
	m_logger.logOutputLine(fullMessage);
}


bool ModelProcessor::validateFilePath(const std::string& filePath) const
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
