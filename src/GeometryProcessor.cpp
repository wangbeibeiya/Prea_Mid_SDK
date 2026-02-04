#include "GeometryProcessor.h"
#include <base/pfGroupData.h>
#include <base/pfDocument.h>
#include <geometry/pfGeometryData.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <json.hpp>
using json = nlohmann::json;

// 匹配统计信息结构体
namespace {
	struct MatchStatistics {
		int totalVolumes = 0;
		int matchedVolumes = 0;
		int renamedVolumes = 0;
		std::vector<std::pair<std::string, std::string>> volumeRenames;
		
		int totalFaces = 0;
		int matchedFaces = 0;
		std::unordered_map<std::string, int> faceMatchesBySet;
	};
}

GeometryProcessor::GeometryProcessor(GeometryAPI* geometryAPI, Logger* logger)
	: m_geometryAPI(geometryAPI)
	, m_logger(logger)
{
}

GeometryProcessor::~GeometryProcessor()
{
}

void GeometryProcessor::setProgressCallback(std::function<void(const std::string&)> callback)
{
	m_progressCallback = callback;
}

void GeometryProcessor::setError(const std::string& error)
{
	m_lastError = error;
	if (m_logger)
	{
		m_logger->logOutputLine("错误: " + error);
	}
}

void GeometryProcessor::printProgress(const std::string& message)
{
	if (m_logger)
	{
		m_logger->logOutputLine(message);
	}
}

bool GeometryProcessor::analyzeVolumes(ProjectModelData* modelData, 
                                      double tolerance,
                                      GeometryModel* geometryModel,
                                      std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap)
{
	// 统计信息
	MatchStatistics stats;

	try
	{
		printProgress("开始分析体对象信息...");

		// 如果提供了 modelData，准备匹配数据
		// 优化3：使用vector存储，保持顺序以便快速访问
		std::vector<std::pair<BoundingBox3D, std::string>> jsonSolids;
		if (modelData)
		{
			const auto& setList = modelData->GetSetList();
			jsonSolids.reserve(setList.size()); // 预分配容量
			for (const auto& setItem : setList)
			{
				if (setItem && setItem->SetType.has_value() && setItem->SetType.value() == "Solid")
				{
					auto solidSet = std::dynamic_pointer_cast<SetSolidItem>(setItem);
					if (solidSet && !solidSet->Items.empty())
					{
						std::string setName = solidSet->SetName.has_value() ? solidSet->SetName.value() : "";
						const auto& solidItem = solidSet->Items[0];
						jsonSolids.push_back({ solidItem.BoundingBox, setName });
					}
				}
			}
			if (!jsonSolids.empty())
			{
				printProgress("已加载 " + std::to_string(jsonSolids.size()) + " 个 JSON 体数据用于匹配（来自 SetList 中的 Solid 集合）");
			}
		}

		// 用于存储体名称和对应的面组名称映射（如果未提供则创建临时变量）
		std::unordered_map<std::string, std::vector<std::string>> tempVolumeFaceGroupsMap;
		if (!volumeFaceGroupsMap)
		{
			volumeFaceGroupsMap = &tempVolumeFaceGroupsMap;
		}

		// 获取初始几何数据
		PREPRO_BASE_NAMESPACE::PFData initialGeometryData;
		if (!m_geometryAPI->getAllData(initialGeometryData))
		{
			setError("获取几何数据失败: " + m_geometryAPI->getLastError());
			return false;
		}

		// 搜索体对象
		int volumes_num = initialGeometryData.getVolumeSize();
		PREPRO_BASE_NAMESPACE::PFVolume** volumes = initialGeometryData.getVolumes();

		if (m_logger)
		{
			m_logger->logOutputLine("\n======== 体对象分析报告 ========");
			m_logger->logOutputLine("初始体对象数量: " + std::to_string(volumes_num));
		}

		// 清理之前的几何模型数据
		if (geometryModel)
		{
			geometryModel->clear();
		}

		// 用于跟踪哪些面已经匹配到集合中
		std::unordered_map<std::string, std::vector<unsigned int>> matchedFacesBySet;

		// 根据体中的group，按ID分组得到真实面（创建新的面组）
		printProgress("开始创建新的面组（按ID分组）...");
		for (int i = 0; i < volumes_num; i++)
		{
			PREPRO_BASE_NAMESPACE::PFVolume* vol = volumes[i];
			if (!vol) continue;

			int groups_num = vol->getGroupSize();
			PREPRO_BASE_NAMESPACE::PFGroup** groups = vol->getGroups();

			for (int j = 0; j < groups_num; j++)
			{
				PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
				if (!group) continue;

				char* group_name = group->getName();
				createFaceGroupsInGroup(group, group_name ? group_name : "未命名", modelData, tolerance, &matchedFacesBySet);
			}
		}

		// 重新获取最新数据（必须调用，创建面组后数据结构已改变）
		printProgress("重新获取最新几何数据（创建面组后）...");
		PREPRO_BASE_NAMESPACE::PFData geometryData;
		if (!m_geometryAPI->getAllData(geometryData))
		{
			setError("重新获取几何数据失败: " + m_geometryAPI->getLastError());
			return false;
		}

		volumes_num = geometryData.getVolumeSize();
		volumes = geometryData.getVolumes();

		if (m_logger)
		{
			m_logger->logOutputLine("创建面组后重新获取的体对象数量: " + std::to_string(volumes_num));
		}

		if (volumes_num == 0)
		{
			if (m_logger)
			{
				m_logger->logOutputLine("⚠ 警告: 创建面组后体对象数量为0！");
				m_logger->logOutputLine("  尝试使用初始数据继续处理...");
			}
			volumes_num = initialGeometryData.getVolumeSize();
			volumes = initialGeometryData.getVolumes();
		}

		// 遍历所有面组，删除elementSize为0的面组
		printProgress("检查并删除空面组...");
		std::vector<std::string> emptyGroupNamesToDelete;
		for (int i = 0; i < volumes_num; i++)
		{
			PREPRO_BASE_NAMESPACE::PFVolume* vol = volumes[i];
			if (!vol) continue;

			int groups_num = vol->getGroupSize();
			PREPRO_BASE_NAMESPACE::PFGroup** groups = vol->getGroups();

			for (int j = 0; j < groups_num; j++)
			{
				PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
				if (!group) continue;

				size_t elementSize = group->getElementSize();
				char* group_name = group->getName();

				if (elementSize == 0 && group_name && strlen(group_name) > 0)
				{
					emptyGroupNamesToDelete.push_back(std::string(group_name));
				}
			}
		}

		if (!emptyGroupNamesToDelete.empty())
		{
			printProgress("正在删除 " + std::to_string(emptyGroupNamesToDelete.size()) + " 个空面组...");
			if (m_geometryAPI->deleteFaceGroups(emptyGroupNamesToDelete))
			{
				printProgress("成功删除 " + std::to_string(emptyGroupNamesToDelete.size()) + " 个空面组");

				// 删除后再次重新获取数据（必须调用，删除面组后数据结构已改变）
				if (!m_geometryAPI->getAllData(geometryData))
				{
					setError("删除空面组后重新获取几何数据失败: " + m_geometryAPI->getLastError());
					return false;
				}
				volumes_num = geometryData.getVolumeSize();
				volumes = geometryData.getVolumes();
			}
			else
			{
				printProgress("删除空面组失败: " + m_geometryAPI->getLastError());
			}
		}

			// 开始分析体和面（优化：边遍历边计算包围盒，不存储所有顶点，消除重复计算）
		printProgress("开始分析体和面...");
		stats.totalVolumes = volumes_num;
		
		// 优化2：减少日志输出，只在开始时输出一次分隔符
		if (m_logger && volumes_num <= 10) // 只有体数量少时才输出详细分隔符
		{
			m_logger->logOutputLine("\n======== 开始分析 " + std::to_string(volumes_num) + " 个体对象 ========");
		}
		
		for (int i = 0; i < volumes_num; i++)
		{
			// 优化2：减少日志输出
			if (m_logger && volumes_num <= 10)
			{
				m_logger->logOutputLine("\n--------------------------");
			}

			PREPRO_BASE_NAMESPACE::PFVolume* vol = volumes[i];
			char* vol_name = vol->getName();
			int vol_id = vol->getId();

			// 优化：边遍历边计算包围盒，不存储所有顶点，避免重复计算
			int groups_num = vol->getGroupSize();
			PREPRO_BASE_NAMESPACE::PFGroup** groups = vol->getGroups();
			
			bool hasVertices = false;
			double min_x = 0.0, max_x = 0.0, min_y = 0.0, max_y = 0.0, min_z = 0.0, max_z = 0.0;
			bool firstVertex = true;

			// 遍历所有组，边遍历边计算包围盒
			for (int j = 0; j < groups_num; j++)
			{
				PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
				if (!group) continue;

				size_t vertex_count = group->getVertexSize();
				double* vertexs = group->getVertexes();

				for (size_t k = 0; k < vertex_count * 3; k += 3)
				{
					double x = vertexs[k];
					double y = vertexs[k + 1];
					double z = vertexs[k + 2];

					if (firstVertex)
					{
						min_x = max_x = x;
						min_y = max_y = y;
						min_z = max_z = z;
						firstVertex = false;
						hasVertices = true;
					}
					else
					{
						min_x = std::min(min_x, x);
						max_x = std::max(max_x, x);
						min_y = std::min(min_y, y);
						max_y = std::max(max_y, y);
						min_z = std::min(min_z, z);
						max_z = std::max(max_z, z);
					}
				}
			}

			// 计算包围盒并匹配（优化：合并计算，避免重复调用calculateBoundingBoxAndCentroid）
			BoundingBox3D volumeBBox;
			if (hasVertices)
			{
				volumeBBox.minPoint = Point3D(min_x, min_y, min_z);
				volumeBBox.maxPoint = Point3D(max_x, max_y, max_z);
				volumeBBox.center = Point3D((min_x + max_x) / 2.0, (min_y + max_y) / 2.0, (min_z + max_z) / 2.0);
				volumeBBox.size = Point3D(max_x - min_x, max_y - min_y, max_z - min_z);

				// 优化2：减少日志输出，只在体数量少或匹配成功时输出详细信息
				if (m_logger && (volumes_num <= 10 || modelData))
				{
					m_logger->logOutputLine("体对象 #" + std::to_string(i + 1) + " (ID: " + std::to_string(vol_id) + ", 名称: " + (vol_name ? std::string(vol_name) : "未命名") + ")");
					if (volumes_num <= 5) // 只有体数量很少时才输出详细包围盒信息
					{
						m_logger->logOutputLine("  包围盒信息:");
						m_logger->logOutputLine("    中心点(Center): (" + std::to_string(volumeBBox.center.x) + ", " + std::to_string(volumeBBox.center.y) + ", " + std::to_string(volumeBBox.center.z) + ")");
						m_logger->logOutputLine("    尺寸(Size): " + std::to_string(volumeBBox.size.x) + " × " + std::to_string(volumeBBox.size.y) + " × " + std::to_string(volumeBBox.size.z));
						m_logger->logOutputLine("    边界范围: [" + std::to_string(min_x) + "~" + std::to_string(max_x) + ", " + std::to_string(min_y) + "~" + std::to_string(max_y) + ", " + std::to_string(min_z) + "~" + std::to_string(max_z) + "]");
					}
				}

				// 根据包围盒与JSON数据中的体去匹配，匹配成功改名字
				if (modelData && !jsonSolids.empty())
				{
					std::string currentName = vol_name ? std::string(vol_name) : "";
					std::string matchedName;
					bool foundMatch = false;
					
					// 遍历所有JSON体进行匹配（撤销提前退出策略）
					for (const auto& jsonSolid : jsonSolids)
					{
						const BoundingBox3D& jsonBBox = jsonSolid.first;
						const std::string& candidateName = jsonSolid.second;

						if (matchBoundingBox(volumeBBox, jsonBBox, tolerance))
						{
							matchedName = candidateName;
							foundMatch = true;
							stats.matchedVolumes++;
							// 优化2：减少日志输出，只在匹配成功时输出
							if (m_logger)
							{
								m_logger->logOutputLine("  ✓ 匹配成功: \"" + matchedName + "\"");
							}

							if (currentName != matchedName)
							{
								if (!currentName.empty())
								{
									char* actualVolName = vol->getName();
									std::string actualName = actualVolName ? std::string(actualVolName) : "";
									if (actualName != currentName)
									{
										currentName = actualName;
									}
									
									// 检查是否存在同名体积（避免名称冲突）
									bool nameConflict = false;
									for (int k = 0; k < volumes_num; k++)
									{
										if (k != i)
										{
											PREPRO_BASE_NAMESPACE::PFVolume* otherVol = volumes[k];
											if (otherVol)
											{
												char* otherName = otherVol->getName();
												if (otherName && std::string(otherName) == matchedName)
												{
													nameConflict = true;
													if (m_logger)
													{
														m_logger->logOutputLine("  ⚠ 警告: 目标名称已存在，跳过重命名");
													}
													break;
												}
											}
										}
									}
									
									if (!nameConflict)
									{
										if (m_geometryAPI->renameVolume(currentName, matchedName))
										{
											if (m_logger)
											{
												m_logger->logOutputLine("  ✓ 重命名成功: \"" + currentName + "\" -> \"" + matchedName + "\"");
											}
											stats.renamedVolumes++;
											stats.volumeRenames.push_back({ currentName, matchedName });
										}
										else
										{
											if (m_logger)
											{
												m_logger->logOutputLine("  ✗ 重命名失败: " + m_geometryAPI->getLastError());
											}
										}
									}
								}
								else
								{
									std::string tempName = "Volume_" + std::to_string(vol_id);
									if (m_geometryAPI->renameVolume(tempName, matchedName))
									{
										if (m_logger)
										{
											m_logger->logOutputLine("  ✓ 重命名成功: \"" + tempName + "\" -> \"" + matchedName + "\"");
										}
										stats.renamedVolumes++;
										stats.volumeRenames.push_back({ tempName, matchedName });
									}
									else
									{
										if (m_logger)
										{
											m_logger->logOutputLine("  ✗ 重命名失败: " + m_geometryAPI->getLastError());
										}
									}
								}
							}
							// 撤销提前退出策略：继续遍历所有JSON体
						}
					}

					// 优化2：减少日志输出，只在体数量少时输出未匹配信息
					if (!foundMatch && m_logger && volumes_num <= 10)
					{
						m_logger->logOutputLine("  ✗ 未找到匹配的JSON体");
					}
				}
			}

			// 然后分析真实面：遍历每个组，分析其中的真实面
			std::vector<std::string> volumeFaceGroupNames;
			volumeFaceGroupNames.reserve(groups_num * 10); // 优化3：预分配容量
			for (int j = 0; j < groups_num; j++)
			{
				PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
				if (!group) continue;

				char* group_name = group->getName();
				int group_id = group->getId();

				// 优化2：减少日志输出，只在组数量少时输出
				if (m_logger && groups_num <= 10)
				{
					m_logger->logOutputLine("  分析组: " + (group_name ? std::string(group_name) : "未命名") + " (ID: " + std::to_string(group_id) + ")");
				}
				std::vector<std::string> faceGroupNames = analyzeFacesInGroupAndGetNames(group, j + 1, group_name ? group_name : "未命名", modelData, tolerance, &stats);
				volumeFaceGroupNames.insert(volumeFaceGroupNames.end(), faceGroupNames.begin(), faceGroupNames.end());
			}
			
			// 获取最终的体名称（可能已被重命名）
			char* finalVolName = vol->getName();
			std::string volumeName = finalVolName ? std::string(finalVolName) : "";
			if (volumeName.empty())
			{
				volumeName = "Volume_" + std::to_string(vol_id);
			}
			
			// 去重面组名称
			std::set<std::string> uniqueFaceGroups(volumeFaceGroupNames.begin(), volumeFaceGroupNames.end());
			volumeFaceGroupNames.assign(uniqueFaceGroups.begin(), uniqueFaceGroups.end());
			
			// 存储到映射表中
			if (!volumeFaceGroupNames.empty())
			{
				(*volumeFaceGroupsMap)[volumeName] = volumeFaceGroupNames;
			}

			// 创建几何体数据结构对象
			if (geometryModel)
			{
				auto geometryVolume = createGeometryVolume(vol, i);
				if (geometryVolume)
				{
					geometryModel->addVolume(geometryVolume);
					// 优化2：减少日志输出
					if (m_logger && volumes_num <= 10)
					{
						m_logger->logOutputLine("  ✓ 几何体数据结构已创建，包含 " + std::to_string(geometryVolume->getFaceCount()) + " 个面");
					}
				}
			}
		}

		// 如果执行了重命名操作，需要重新获取几何数据以刷新体对象名称
		if (stats.renamedVolumes > 0)
		{
			printProgress("重新获取最新几何数据（重命名操作后）...");
			if (!m_geometryAPI->getAllData(geometryData))
			{
				setError("重命名后重新获取几何数据失败: " + m_geometryAPI->getLastError());
				return false;
			}
			volumes_num = geometryData.getVolumeSize();
			volumes = geometryData.getVolumes();
			
			if (m_logger)
			{
				m_logger->logOutputLine("重命名后重新获取的体对象数量: " + std::to_string(volumes_num));
			}
		}

		// 如果提供了modelData，将重命名映射保存到ProjectModelData中，供后续使用（例如边界层设置）
		if (modelData && !stats.volumeRenames.empty())
		{
			for (const auto& renamePair : stats.volumeRenames)
			{
				modelData->addVolumeRename(renamePair.first, renamePair.second);
			}
		}

		// 输出匹配和重命名总结
		if (m_logger)
		{
			m_logger->logOutputLine("");
			m_logger->logOutputLine("╔════════════════════════════════════════════════════════════╗");
			m_logger->logOutputLine("║              匹配和重命名总结报告                          ║");
			m_logger->logOutputLine("╚════════════════════════════════════════════════════════════╝");
			
			m_logger->logOutputLine("");
			m_logger->logOutputLine("【体对象匹配统计】");
			m_logger->logOutputLine("  ┌─────────────────────────────────────────────┐");
			m_logger->logOutputLine("  │ 总体对象数量:        " + std::to_string(stats.totalVolumes) + " 个        │");
			m_logger->logOutputLine("  │ 匹配成功的体数量:    " + std::to_string(stats.matchedVolumes) + " 个        │");
			m_logger->logOutputLine("  │ 重命名成功的体数量:  " + std::to_string(stats.renamedVolumes) + " 个        │");
			if (stats.totalVolumes > 0)
			{
				double matchRate = (stats.matchedVolumes * 100.0) / stats.totalVolumes;
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(1) << matchRate;
				m_logger->logOutputLine("  │ 匹配成功率:          " + oss.str() + " %        │");
			}
			m_logger->logOutputLine("  └─────────────────────────────────────────────┘");
			
			if (!stats.volumeRenames.empty())
			{
				m_logger->logOutputLine("");
				m_logger->logOutputLine("  ✓ 体重命名详情 (" + std::to_string(stats.volumeRenames.size()) + " 个体):");
				for (size_t i = 0; i < stats.volumeRenames.size(); i++)
				{
					const auto& rename = stats.volumeRenames[i];
					std::ostringstream oss;
					oss << "    [" << std::setw(2) << (i + 1) << "] \"" << rename.first << "\" → \"" << rename.second << "\"";
					m_logger->logOutputLine(oss.str());
				}
			}
			
			m_logger->logOutputLine("");
			m_logger->logOutputLine("【面对象匹配统计】");
			m_logger->logOutputLine("  ┌─────────────────────────────────────────────┐");
			m_logger->logOutputLine("  │ 总面数量:            " + std::to_string(stats.totalFaces) + " 个        │");
			m_logger->logOutputLine("  │ 匹配成功的面数量:    " + std::to_string(stats.matchedFaces) + " 个        │");
			if (stats.totalFaces > 0)
			{
				double faceMatchRate = (stats.matchedFaces * 100.0) / stats.totalFaces;
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(1) << faceMatchRate;
				m_logger->logOutputLine("  │ 匹配成功率:          " + oss.str() + " %        │");
			}
			m_logger->logOutputLine("  └─────────────────────────────────────────────┘");
			
			if (!stats.faceMatchesBySet.empty())
			{
				m_logger->logOutputLine("");
				m_logger->logOutputLine("  ✓ 按集合统计匹配的面数量 (" + std::to_string(stats.faceMatchesBySet.size()) + " 个集合):");
				int setIndex = 1;
				for (const auto& pair : stats.faceMatchesBySet)
				{
					std::ostringstream oss;
					oss << "    [" << std::setw(2) << setIndex++ << "] 集合 \"" << pair.first << "\": " << pair.second << " 个面";
					m_logger->logOutputLine(oss.str());
				}
			}
			
			m_logger->logOutputLine("");
			m_logger->logOutputLine("╔════════════════════════════════════════════════════════════╗");
			m_logger->logOutputLine("║                    总结报告结束                              ║");
			m_logger->logOutputLine("╚════════════════════════════════════════════════════════════╝");
			m_logger->logOutputLine("");
		}

		// 输出几何模型统计信息
		if (geometryModel)
		{
			geometryModel->printSummary();
		}

		return true;
	}
	catch (const std::exception& e)
	{
		setError("分析体对象时发生异常: " + std::string(e.what()));
		return false;
	}
}

bool GeometryProcessor::matchBoundingBox(const BoundingBox3D& bbox1, const BoundingBox3D& bbox2, double tolerance)
{
	// 比较两个包围盒的min点和max点是否都在容差范围内
	bool minMatch = (std::abs(bbox1.minPoint.x - bbox2.minPoint.x) < tolerance) &&
		(std::abs(bbox1.minPoint.y - bbox2.minPoint.y) < tolerance) &&
		(std::abs(bbox1.minPoint.z - bbox2.minPoint.z) < tolerance);
	
	bool maxMatch = (std::abs(bbox1.maxPoint.x - bbox2.maxPoint.x) < tolerance) &&
		(std::abs(bbox1.maxPoint.y - bbox2.maxPoint.y) < tolerance) &&
		(std::abs(bbox1.maxPoint.z - bbox2.maxPoint.z) < tolerance);
	
	return minMatch && maxMatch;
}

void GeometryProcessor::createFaceGroupsInGroup(PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                                 const std::string& groupName, 
                                                 const ProjectModelData* modelData, 
                                                 double tolerance,
                                                 std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet)
{
	if (!group)
	{
		return;
	}

	try
	{
		size_t elementSize = group->getElementSize();
		PREPRO_BASE_NAMESPACE::PFElement** elements = group->getElements();

		if (elementSize == 0 || !elements)
		{
			return;
		}

		// 按ID分组元素
		std::unordered_map<unsigned int, std::vector<PREPRO_BASE_NAMESPACE::PFElement*>> faceElementsMap = groupElementsById(elements, elementSize);

		// 优化2：减少日志输出
		if (m_logger && faceElementsMap.size() <= 20)
		{
			m_logger->logOutputLine("      识别出的真实面数量（按ID分组）: " + std::to_string(faceElementsMap.size()));
		}

		// 遍历每个真实面，逐个创建面组
		int createdCount = 0;
		int failedCount = 0;
		for (const auto& pair : faceElementsMap)
		{
			unsigned int faceId = pair.first;

			// 生成group名称：原始组名_Face_面ID
			std::string baseGroupName = (groupName == "未命名" || groupName.empty()) ?
				("Group_" + std::to_string(group->getId())) : groupName;
			std::string newGroupName = baseGroupName + "_Face_" + std::to_string(faceId);

			// 创建新 group（单个创建）
			std::vector<unsigned int> faceIds = { faceId };
			if (m_geometryAPI->addEntitiesToGroup(faceIds, newGroupName))
			{
				createdCount++;
			}
			else
			{
				failedCount++;
				if (m_logger)
				{
					m_logger->logOutputLine("  ✗ 创建面组失败: " + newGroupName + " (" + m_geometryAPI->getLastError() + ")");
				}
			}
		}
		
		if (m_logger && (createdCount > 0 || failedCount > 0))
		{
			m_logger->logOutputLine("  创建面组统计: 成功 " + std::to_string(createdCount) + " 个, 失败 " + std::to_string(failedCount) + " 个");
		}
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("  错误: 创建面组时发生异常: " + std::string(e.what()));
		}
	}
}

void GeometryProcessor::analyzeRealFacesInGroup(PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                                int groupIndex, 
                                                const std::string& groupName, 
                                                const ProjectModelData* modelData, 
                                                double tolerance,
                                                GeometryModel* geometryModel,
                                                std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet,
                                                std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap)
{
	// 调用完整实现函数
	analyzeRealFacesInGroupImpl(group, nullptr, groupIndex, groupName, modelData, tolerance, nullptr, nullptr, geometryModel, matchedFacesBySet, volumeFaceGroupsMap);
}

void GeometryProcessor::analyzeRealFacesInGroupImpl(PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                                    PREPRO_BASE_NAMESPACE::PFVolume* volume, 
                                                    int groupIndex, 
                                                    const std::string& groupName, 
                                                    const ProjectModelData* modelData, 
                                                    double tolerance, 
                                                    void* statsPtr, 
                                                    std::vector<std::string>* faceGroupNames,
                                                    GeometryModel* geometryModel,
                                                    std::unordered_map<std::string, std::vector<unsigned int>>* matchedFacesBySet,
                                                    std::unordered_map<std::string, std::vector<std::string>>* volumeFaceGroupsMap)
{
	if (!group)
	{
		return;
	}

	// 获取统计信息指针
	struct MatchStatistics* stats = static_cast<struct MatchStatistics*>(statsPtr);

	try
	{
		// 如果提供了 modelData，准备匹配数据（筛选 SetType 为 "Face" 的集合）
		// 优化3：预分配容量，减少内存重新分配
		std::vector<std::pair<std::string, std::vector<BoundingBox3D>>> faceSets;
		if (modelData)
		{
			const auto& setList = modelData->GetSetList();
			faceSets.reserve(setList.size()); // 预分配容量
			for (const auto& setItem : setList)
			{
				if (!setItem) continue;

				if (setItem->SetType.has_value() && setItem->SetType.value() == "Face")
				{
					auto faceSet = std::dynamic_pointer_cast<SetFaceItem>(setItem);
					if (faceSet && faceSet->SetName.has_value())
					{
						std::string setName = faceSet->SetName.value();
						std::vector<BoundingBox3D> bboxes;
						bboxes.reserve(faceSet->Items.size()); // 预分配容量

						for (const auto& faceItem : faceSet->Items)
						{
							bboxes.push_back(faceItem.BoundingBox);
						}

						if (!bboxes.empty())
						{
							faceSets.push_back({ setName, std::move(bboxes) }); // 使用move避免拷贝
						}
					}
				}
			}
		}

		// 获取组中的所有元素
		size_t elementSize = group->getElementSize();
		PREPRO_BASE_NAMESPACE::PFElement** elements = group->getElements();

		if (elementSize == 0 || !elements)
		{
			return;
		}

		PREPRO_BASE_NAMESPACE::PFGroup* vertexGroup = group;

		// 按ID分组元素
		std::unordered_map<unsigned int, std::vector<PREPRO_BASE_NAMESPACE::PFElement*>> faceElementsMap = groupElementsById(elements, elementSize);
	
		if (stats) stats->totalFaces += static_cast<int>(faceElementsMap.size());

		// 存储当前组的面数据
		std::vector<std::shared_ptr<GeometryFace>> currentGroupFaces;

		// 用于跟踪哪些面已经匹配到集合中
		std::unordered_map<std::string, std::vector<unsigned int>> matchedFacesBySetLocal;
		
		// 用于跟踪每个集合中哪些面（索引）已经被匹配了
		// key: 集合名称, value: 已匹配的面在集合中的索引集合
		std::unordered_map<std::string, std::set<size_t>> matchedFaceIndicesBySet;

		// 优化3：预处理JSON数据，建立快速查找索引（按集合名称索引，避免重复查找）
		std::unordered_map<std::string, std::vector<std::pair<size_t, BoundingBox3D>>> faceSetIndex;
		if (modelData && !faceSets.empty())
		{
			for (const auto& faceSet : faceSets)
			{
				const std::string& setName = faceSet.first;
				const std::vector<BoundingBox3D>& jsonBBoxes = faceSet.second;
				faceSetIndex[setName].reserve(jsonBBoxes.size()); // 预分配容量
				
				for (size_t idx = 0; idx < jsonBBoxes.size(); ++idx)
				{
					faceSetIndex[setName].push_back({ idx, jsonBBoxes[idx] });
				}
			}
		}

		// 统计信息
		int createdCount = 0;
		int failedCount = 0;

		// 辅助函数：检查是否所有JSON面集合的所有面都已匹配完成
		auto allSetsMatched = [&faceSetIndex, &matchedFaceIndicesBySet]() -> bool {
			for (const auto& faceSetPair : faceSetIndex)
			{
				const std::string& setName = faceSetPair.first;
				size_t totalFaces = faceSetPair.second.size();
				size_t matchedCount = matchedFaceIndicesBySet[setName].size();
				if (matchedCount < totalFaces)
				{
					return false; // 还有未匹配的面
				}
			}
			return true; // 所有集合的所有面都已匹配
		};

		// 分析每个真实面
		for (const auto& pair : faceElementsMap)
		{
			// 优化：如果所有JSON面集合的所有面都已匹配完成，提前退出
			if (modelData && !faceSetIndex.empty() && allSetsMatched())
			{
				if (m_logger)
				{
					m_logger->logOutputLine("  所有JSON面集合的所有面都已匹配完成，提前退出匹配循环");
				}
				break;
			}

			unsigned int faceId = pair.first;
			const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements = pair.second;

			// 优化：先收集顶点，避免重复调用
			std::set<unsigned int> uniqueVertexIndices = collectUniqueVertexIndices(faceElements);
			std::vector<std::vector<double>> faceVertices;
			
			if (!getVerticesFromGroup(uniqueVertexIndices, vertexGroup, faceVertices) || faceVertices.empty())
			{
				continue;
			}

			// 计算面的包围盒用于匹配（优化：提取为统一的计算逻辑）
			BoundingBox3D faceBBox;
			if (!faceVertices.empty())
			{
				double min_x = faceVertices[0][0], max_x = faceVertices[0][0];
				double min_y = faceVertices[0][1], max_y = faceVertices[0][1];
				double min_z = faceVertices[0][2], max_z = faceVertices[0][2];

				for (const auto& vertex : faceVertices)
				{
					min_x = std::min(min_x, vertex[0]);
					max_x = std::max(max_x, vertex[0]);
					min_y = std::min(min_y, vertex[1]);
					max_y = std::max(max_y, vertex[1]);
					min_z = std::min(min_z, vertex[2]);
					max_z = std::max(max_z, vertex[2]);
				}

				faceBBox.minPoint = Point3D(min_x, min_y, min_z);
				faceBBox.maxPoint = Point3D(max_x, max_y, max_z);
				faceBBox.center = Point3D((min_x + max_x) / 2.0, (min_y + max_y) / 2.0, (min_z + max_z) / 2.0);
				faceBBox.size = Point3D(max_x - min_x, max_y - min_y, max_z - min_z);
			}

			// 计算该真实面的属性（使用已收集的顶点数据，避免重复收集）
			// 优化2：减少日志输出，只在必要时输出
			if (m_logger && faceElementsMap.size() <= 10) // 只有面数量少时才输出详细日志
			{
				calculateRealFaceProperties(faceElements, faceId, vertexGroup);
			}

			// 优化3：使用预处理索引进行快速匹配
			std::string matchedSetName;
			size_t matchedFaceIndex = SIZE_MAX;
			if (modelData && !faceSetIndex.empty() && !faceVertices.empty())
			{
				for (auto& faceSetPair : faceSetIndex)
				{
					const std::string& setName = faceSetPair.first;
					std::vector<std::pair<size_t, BoundingBox3D>>& indexedFaces = faceSetPair.second;
					
					// 优化：提前获取引用，避免重复查找
					std::set<size_t>& matchedIndices = matchedFaceIndicesBySet[setName];
					
					// 优化：如果集合中所有面都已匹配，跳过该集合
					if (matchedIndices.size() >= indexedFaces.size())
					{
						continue;
					}

					// 遍历集合中的所有面，只匹配未匹配的面
					for (auto& indexedFace : indexedFaces)
					{
						size_t idx = indexedFace.first;
						const BoundingBox3D& jsonBBox = indexedFace.second;
						
						// 如果这个面已经被匹配过了，跳过
						if (matchedIndices.find(idx) != matchedIndices.end())
						{
							continue;
						}

						bool matches = matchBoundingBox(faceBBox, jsonBBox, tolerance);
						
						if (matches)
						{
							matchedSetName = setName;
							matchedFaceIndex = idx;
							// 标记这个面已经被匹配
							matchedIndices.insert(idx);
							
							if (stats)
							{
								stats->matchedFaces++;
								stats->faceMatchesBySet[setName]++;
							}
							
							// 匹配成功后，只匹配当前集合中的一个面，然后退出内层循环
							// 这样可以确保一个真实面只匹配到一个集合的一个面
							break;
						}
					}
					
					// 如果已经匹配成功，退出外层循环（不再检查其他集合）
					if (!matchedSetName.empty())
					{
						break;
					}
				}
			}

			// 只有匹配成功时才创建面组
			if (!matchedSetName.empty())
			{
				// 匹配成功：使用集合名称作为组名
				std::string newGroupName = matchedSetName;
				matchedFacesBySetLocal[matchedSetName].push_back(faceId);
				
				// 立即创建面组（单个创建，不使用批量优化）
				std::vector<unsigned int> faceIds = { faceId };
				if (m_geometryAPI->addEntitiesToGroup(faceIds, newGroupName))
				{
					createdCount++;
					if (faceGroupNames)
					{
						faceGroupNames->push_back(newGroupName);
					}
				}
				else
				{
					failedCount++;
					if (m_logger)
					{
						m_logger->logOutputLine("  ✗ 创建面组失败: " + newGroupName + " (" + m_geometryAPI->getLastError() + ")");
					}
				}
			}
			// 匹配失败：不创建面组，跳过该面

			// 创建几何面数据结构对象
			if (geometryModel)
			{
				auto geometryFace = createGeometryFace(faceElements, faceId, group, faceElements.size());
				if (geometryFace)
				{
					geometryFace->setGroupIndex(groupIndex);
					currentGroupFaces.push_back(geometryFace);
				}
			}
		}

		// 输出创建统计
		if (m_logger && (createdCount > 0 || failedCount > 0))
		{
			m_logger->logOutputLine("  创建面组统计: 成功 " + std::to_string(createdCount) + " 个, 失败 " + std::to_string(failedCount) + " 个");
		}

		// 输出匹配统计，并检查每个集合是否所有面都被匹配
		if (m_logger)
		{
			m_logger->logOutputLine("  面匹配统计:");
			for (const auto& faceSet : faceSets)
			{
				const std::string& setName = faceSet.first;
				const std::vector<BoundingBox3D>& jsonBBoxes = faceSet.second;
				size_t totalFacesInSet = jsonBBoxes.size();
				size_t matchedCount = matchedFaceIndicesBySet[setName].size();
				
				if (matchedCount > 0)
				{
					m_logger->logOutputLine("    集合 \"" + setName + "\": " + std::to_string(matchedCount) + "/" + std::to_string(totalFacesInSet) + " 个面已匹配");
					
					// 检查是否所有面都被匹配
					if (matchedCount == totalFacesInSet)
					{
						m_logger->logOutputLine("      ✓ 集合 \"" + setName + "\" 所有面匹配成功！");
					}
					else
					{
						m_logger->logOutputLine("      ⚠ 警告: 集合 \"" + setName + "\" 还有 " + std::to_string(totalFacesInSet - matchedCount) + " 个面未匹配");
					}
				}
				else if (totalFacesInSet > 0) // 优化2：只输出有未匹配面的集合
				{
					m_logger->logOutputLine("    集合 \"" + setName + "\": 0/" + std::to_string(totalFacesInSet) + " 个面已匹配（未匹配）");
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("  错误: 分析真实面时发生异常: " + std::string(e.what()));
		}
	}
}

void GeometryProcessor::calculateBoundingBoxAndCentroid(const std::vector<std::vector<double>>& vertices, int volumeIndex, GeometryModel* geometryModel)
{
	if (vertices.empty())
	{
		return;
	}

	// 初始化包围盒
	double min_x = vertices[0][0], max_x = vertices[0][0];
	double min_y = vertices[0][1], max_y = vertices[0][1];
	double min_z = vertices[0][2], max_z = vertices[0][2];

	// 遍历所有顶点计算包围盒
	for (const auto& vertex : vertices)
	{
		double x = vertex[0];
		double y = vertex[1];
		double z = vertex[2];

		min_x = std::min(min_x, x);
		max_x = std::max(max_x, x);
		min_y = std::min(min_y, y);
		max_y = std::max(max_y, y);
		min_z = std::min(min_z, z);
		max_z = std::max(max_z, z);
	}

	// 计算包围盒尺寸和中心点
	double width = max_x - min_x;
	double height = max_y - min_y;
	double depth = max_z - min_z;

	double center_x = (min_x + max_x) / 2.0;
	double center_y = (min_y + max_y) / 2.0;
	double center_z = (min_z + max_z) / 2.0;

	if (m_logger)
	{
		m_logger->logOutputLine("  包围盒信息:");
		m_logger->logOutputLine("    中心点(Center): (" + std::to_string(center_x) + ", " + std::to_string(center_y) + ", " + std::to_string(center_z) + ")");
		m_logger->logOutputLine("    尺寸(Size): " + std::to_string(width) + " × " + std::to_string(height) + " × " + std::to_string(depth));
		m_logger->logOutputLine("    边界范围: [" + std::to_string(min_x) + "~" + std::to_string(max_x) + ", " + std::to_string(min_y) + "~" + std::to_string(max_y) + ", " + std::to_string(min_z) + "~" + std::to_string(max_z) + "]");
	}
}

void GeometryProcessor::calculateFaceBoundingBoxAndCentroid(const std::vector<std::vector<double>>& vertices, int faceIndex, const std::string& faceName, GeometryModel* geometryModel)
{
	if (vertices.empty())
	{
		return;
	}

	// 初始化包围盒
	double min_x = vertices[0][0], max_x = vertices[0][0];
	double min_y = vertices[0][1], max_y = vertices[0][1];
	double min_z = vertices[0][2], max_z = vertices[0][2];

	// 遍历所有顶点计算包围盒
	for (const auto& vertex : vertices)
	{
		double x = vertex[0];
		double y = vertex[1];
		double z = vertex[2];

		min_x = std::min(min_x, x);
		max_x = std::max(max_x, x);
		min_y = std::min(min_y, y);
		max_y = std::max(max_y, y);
		min_z = std::min(min_z, z);
		max_z = std::max(max_z, z);
	}

	// 计算包围盒尺寸和中心点
	double width = max_x - min_x;
	double height = max_y - min_y;
	double depth = max_z - min_z;

	double center_x = (min_x + max_x) / 2.0;
	double center_y = (min_y + max_y) / 2.0;
	double center_z = (min_z + max_z) / 2.0;

	if (m_logger)
	{
		m_logger->logOutputLine("      面包围盒信息:");
		m_logger->logOutputLine("        中心点(Center): (" + std::to_string(center_x) + ", " + std::to_string(center_y) + ", " + std::to_string(center_z) + ")");
		m_logger->logOutputLine("        尺寸(Size): " + std::to_string(width) + " × " + std::to_string(height) + " × " + std::to_string(depth));
		m_logger->logOutputLine("        边界范围: [" + std::to_string(min_x) + "~" + std::to_string(max_x) + ", " + std::to_string(min_y) + "~" + std::to_string(max_y) + ", " + std::to_string(min_z) + "~" + std::to_string(max_z) + "]");
	}
}

std::set<unsigned int> GeometryProcessor::collectUniqueVertexIndices(const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements)
{
	std::set<unsigned int> uniqueVertexIndices;
	
	for (PREPRO_BASE_NAMESPACE::PFElement* element : faceElements)
	{
		if (!element) continue;
		
		unsigned int vertexSize = element->getVertexSize();
		unsigned int* vertexIndices = element->getVertexes();
		
		if (vertexIndices)
		{
			for (unsigned int i = 0; i < vertexSize; i++)
			{
				uniqueVertexIndices.insert(vertexIndices[i]);
			}
		}
	}
	
	return uniqueVertexIndices;
}

bool GeometryProcessor::getVerticesFromGroup(const std::set<unsigned int>& vertexIndices, 
                                             PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                             std::vector<std::vector<double>>& outputVertices)
{
	if (!group)
	{
		return false;
	}
	
	size_t groupVertexCount = group->getVertexSize();
	double* groupVertexs = group->getVertexes();
	
	if (!groupVertexs || groupVertexCount == 0)
	{
		return false;
	}
	
	outputVertices.clear();
	int validCount = 0;
	int invalidCount = 0;
	
	for (unsigned int vertexIndex : vertexIndices)
	{
		if (vertexIndex < groupVertexCount)
		{
			size_t baseIndex = vertexIndex * 3;
			size_t arrayLength = groupVertexCount * 3;
			if (baseIndex + 2 < arrayLength)
			{
				outputVertices.push_back({
					groupVertexs[baseIndex],
					groupVertexs[baseIndex + 1],
					groupVertexs[baseIndex + 2]
				});
				validCount++;
			}
			else
			{
				invalidCount++;
			}
		}
		else
		{
			invalidCount++;
		}
	}
	
	if (validCount == 0 && !vertexIndices.empty() && m_logger)
	{
		m_logger->logOutputLine("        ⚠ 警告: 所有顶点索引都无效。索引范围: [0-" + std::to_string(groupVertexCount - 1) + "], 请求的索引数量: " + std::to_string(vertexIndices.size()) + ", group顶点数: " + std::to_string(groupVertexCount));
	}
	else if (invalidCount > 0 && m_logger)
	{
		m_logger->logOutputLine("        ⚠ 警告: 有 " + std::to_string(invalidCount) + " 个无效索引（有效索引范围: [0-" + std::to_string(groupVertexCount - 1) + "]）");
	}
	
	return !outputVertices.empty();
}

bool GeometryProcessor::getVerticesFromGroup(const std::set<unsigned int>& vertexIndices, 
                                             PREPRO_BASE_NAMESPACE::PFGroup* group, 
                                             std::vector<Point3D>& outputVertices)
{
	if (!group)
	{
		return false;
	}
	
	size_t groupVertexCount = group->getVertexSize();
	double* groupVertexs = group->getVertexes();
	
	if (!groupVertexs || groupVertexCount == 0)
	{
		return false;
	}
	
	outputVertices.clear();
	for (unsigned int vertexIndex : vertexIndices)
	{
		if (vertexIndex < groupVertexCount)
		{
			size_t baseIndex = vertexIndex * 3;
			size_t arrayLength = groupVertexCount * 3;
			if (baseIndex + 2 < arrayLength)
			{
				outputVertices.emplace_back(
					groupVertexs[baseIndex],
					groupVertexs[baseIndex + 1],
					groupVertexs[baseIndex + 2]
				);
			}
		}
	}
	
	return !outputVertices.empty();
}

std::unordered_map<unsigned int, std::vector<PREPRO_BASE_NAMESPACE::PFElement*>> GeometryProcessor::groupElementsById(
	PREPRO_BASE_NAMESPACE::PFElement** elements, size_t elementSize)
{
	std::unordered_map<unsigned int, std::vector<PREPRO_BASE_NAMESPACE::PFElement*>> faceElementsMap;
	
	for (size_t i = 0; i < elementSize; i++)
	{
		PREPRO_BASE_NAMESPACE::PFElement* element = elements[i];
		if (element)
		{
			unsigned int elementId = element->getId();
			faceElementsMap[elementId].push_back(element);
		}
	}
	
	return faceElementsMap;
}

std::shared_ptr<GeometryFace> GeometryProcessor::createGeometryFace(
	const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements, 
	int faceId, 
	PREPRO_BASE_NAMESPACE::PFGroup* group, 
	size_t elementCount)
{
	try
	{
		auto geometryFace = std::make_shared<GeometryFace>(faceId);
		geometryFace->setElementCount(elementCount);

		// 收集唯一的顶点索引并从group获取顶点坐标
		std::set<unsigned int> uniqueVertexIndices = collectUniqueVertexIndices(faceElements);
		std::vector<Point3D> faceVertices;
		
		if (!getVerticesFromGroup(uniqueVertexIndices, group, faceVertices))
		{
			if (m_logger)
			{
				m_logger->logOutputLine("          警告: 无法获取该面的顶点数据");
			}
			return geometryFace;
		}

		if (!faceVertices.empty())
		{
			geometryFace->setVertices(faceVertices);
			geometryFace->calculateCentroid();
			geometryFace->calculateBoundingBox();
			geometryFace->calculateArea();
		}

		return geometryFace;
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("          错误: 创建几何面对象时发生异常: " + std::string(e.what()));
		}
		return nullptr;
	}
}

std::shared_ptr<GeometryVolume> GeometryProcessor::createGeometryVolume(
	PREPRO_BASE_NAMESPACE::PFVolume* volume, 
	int volumeIndex)
{
	try
	{
		if (!volume) return nullptr;

		char* vol_name = volume->getName();
		unsigned int vol_id = volume->getId();

		auto geometryVolume = std::make_shared<GeometryVolume>(vol_id, vol_name ? vol_name : "");
		geometryVolume->setIndex(volumeIndex);

		// 收集所有顶点用于计算包围盒
		std::vector<Point3D> allVolumeVertices;
		size_t totalVertexCount = 0;

		// 获取体对象的所有组
		int groups_num = volume->getGroupSize();
		PREPRO_BASE_NAMESPACE::PFGroup** groups = volume->getGroups();

		for (int j = 0; j < groups_num; j++)
		{
			PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
			if (!group) continue;

			// 获取该组的所有顶点
			size_t vertex_count = group->getVertexSize();
			double* vertexs = group->getVertexes();

			for (size_t k = 0; k < vertex_count * 3; k += 3)
			{
				allVolumeVertices.emplace_back(vertexs[k], vertexs[k + 1], vertexs[k + 2]);
			}
			totalVertexCount += vertex_count;

			// 分析该组中的真实面并添加到体对象
			if (group->getElementSize() > 0)
			{
				PREPRO_BASE_NAMESPACE::PFElement** elements = group->getElements();
				std::unordered_map<unsigned int, std::vector<PREPRO_BASE_NAMESPACE::PFElement*>> faceElementsMap = groupElementsById(elements, group->getElementSize());

				for (const auto& pair : faceElementsMap)
				{
					unsigned int faceId = pair.first;
					const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements = pair.second;

					auto geometryFace = createGeometryFace(faceElements, faceId, group, faceElements.size());
					if (geometryFace)
					{
						geometryFace->setGroupIndex(j);
						geometryVolume->addFace(geometryFace);
					}
				}
			}
		}

		// 设置体对象的基本属性
		geometryVolume->setTotalVertexCount(totalVertexCount);

		// 计算包围盒（从所有顶点）
		if (!allVolumeVertices.empty())
		{
			double min_x = allVolumeVertices[0].x, max_x = allVolumeVertices[0].x;
			double min_y = allVolumeVertices[0].y, max_y = allVolumeVertices[0].y;
			double min_z = allVolumeVertices[0].z, max_z = allVolumeVertices[0].z;

			for (const auto& vertex : allVolumeVertices)
			{
				min_x = std::min(min_x, vertex.x);
				max_x = std::max(max_x, vertex.x);
				min_y = std::min(min_y, vertex.y);
				max_y = std::max(max_y, vertex.y);
				min_z = std::min(min_z, vertex.z);
				max_z = std::max(max_z, vertex.z);
			}

			Point3D minPt(min_x, min_y, min_z);
			Point3D maxPt(max_x, max_y, max_z);
			Point3D center((min_x + max_x) / 2.0, (min_y + max_y) / 2.0, (min_z + max_z) / 2.0);
			Point3D size(max_x - min_x, max_y - min_y, max_z - min_z);

			BoundingBox3D bbox(center, size, minPt, maxPt);
			geometryVolume->setBoundingBox(bbox);
			geometryVolume->setCentroid(center);

			// 估算体积（包围盒体积）
			geometryVolume->setVolume(bbox.getVolume());
		}

		return geometryVolume;
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("  错误: 创建几何体对象时发生异常: " + std::string(e.what()));
		}
		return nullptr;
	}
}

void GeometryProcessor::calculateRealFaceProperties(const std::vector<PREPRO_BASE_NAMESPACE::PFElement*>& faceElements, int faceId, PREPRO_BASE_NAMESPACE::PFGroup* group)
{
	if (faceElements.empty())
	{
		if (m_logger)
		{
			m_logger->logOutputLine("          警告: 该面没有元素");
		}
		return;
	}

	try
	{
		std::set<unsigned int> uniqueVertexIndices = collectUniqueVertexIndices(faceElements);
		if (m_logger)
		{
			m_logger->logOutputLine("          收集到的唯一顶点索引数量: " + std::to_string(uniqueVertexIndices.size()));
		}

		std::vector<std::vector<double>> faceVertices;
		if (!getVerticesFromGroup(uniqueVertexIndices, group, faceVertices))
		{
			if (m_logger)
			{
				m_logger->logOutputLine("          警告: 无法获取该面的顶点数据");
				if (group)
				{
					m_logger->logOutputLine("            - group顶点数量: " + std::to_string(group->getVertexSize()));
				}
				m_logger->logOutputLine("            - 唯一顶点索引数量: " + std::to_string(uniqueVertexIndices.size()));
				if (!uniqueVertexIndices.empty())
				{
					m_logger->logOutputLine("            - 索引范围: [" + std::to_string(*uniqueVertexIndices.begin()) + ", " + std::to_string(*uniqueVertexIndices.rbegin()) + "]");
				}
			}
			return;
		}

		if (m_logger)
		{
			m_logger->logOutputLine("          顶点数量: " + std::to_string(faceVertices.size()));
		}

		double min_x = faceVertices[0][0], max_x = faceVertices[0][0];
		double min_y = faceVertices[0][1], max_y = faceVertices[0][1];
		double min_z = faceVertices[0][2], max_z = faceVertices[0][2];

		for (const auto& vertex : faceVertices)
		{
			min_x = std::min(min_x, vertex[0]);
			max_x = std::max(max_x, vertex[0]);
			min_y = std::min(min_y, vertex[1]);
			max_y = std::max(max_y, vertex[1]);
			min_z = std::min(min_z, vertex[2]);
			max_z = std::max(max_z, vertex[2]);
		}

		double center_x = (min_x + max_x) / 2.0;
		double center_y = (min_y + max_y) / 2.0;
		double center_z = (min_z + max_z) / 2.0;
		double size_x = max_x - min_x;
		double size_y = max_y - min_y;
		double size_z = max_z - min_z;

		if (m_logger)
		{
			m_logger->logOutputLine("          包围盒中心: (" + std::to_string(center_x) + ", " + std::to_string(center_y) + ", " + std::to_string(center_z) + ")");
			m_logger->logOutputLine("          包围盒尺寸: (" + std::to_string(size_x) + ", " + std::to_string(size_y) + ", " + std::to_string(size_z) + ")");
			m_logger->logOutputLine("          包围盒范围: Min(" + std::to_string(min_x) + ", " + std::to_string(min_y) + ", " + std::to_string(min_z) + ") ~ Max(" + std::to_string(max_x) + ", " + std::to_string(max_y) + ", " + std::to_string(max_z) + ")");
		}
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("          错误: 计算面属性时发生异常: " + std::string(e.what()));
		}
	}
}

std::vector<std::string> GeometryProcessor::analyzeFacesInGroupAndGetNames(PREPRO_BASE_NAMESPACE::PFGroup* group, int groupIndex, const std::string& groupName, const ProjectModelData* modelData, double tolerance, void* statsPtr)
{
	std::vector<std::string> faceGroupNames;
	if (!group)
	{
		return faceGroupNames;
	}
	
	analyzeRealFacesInGroupImpl(group, nullptr, groupIndex, groupName, modelData, tolerance, statsPtr, &faceGroupNames, nullptr, nullptr, nullptr);
	
	return faceGroupNames;
}

void GeometryProcessor::calculateVolumeCentroid(PREPRO_BASE_NAMESPACE::PFVolume* volume, int volumeIndex)
{
	if (!volume)
	{
		return;
	}

	try
	{
		if (m_logger)
		{
			m_logger->logOutputLine("  体对象质量重心信息:");
		}

		std::vector<std::vector<double>> all_vertices;
		std::vector<std::vector<int>> triangles;

		int groups_num = volume->getGroupSize();
		PREPRO_BASE_NAMESPACE::PFGroup** groups = volume->getGroups();

		for (int j = 0; j < groups_num; j++)
		{
			PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
			if (!group) continue;

			size_t vertex_count = group->getVertexSize();
			double* vertices = group->getVertexes();

			size_t start_index = all_vertices.size();

			for (size_t k = 0; k < vertex_count * 3; k += 3)
			{
				std::vector<double> vertex = { vertices[k], vertices[k + 1], vertices[k + 2] };
				all_vertices.push_back(vertex);
			}

			size_t num_triangles = vertex_count / 9;
			for (size_t t = 0; t < num_triangles; t++)
			{
				std::vector<int> triangle = {
					static_cast<int>(start_index + t * 3),
					static_cast<int>(start_index + t * 3 + 1),
					static_cast<int>(start_index + t * 3 + 2)
				};
				triangles.push_back(triangle);
			}
		}

		if (all_vertices.empty() || triangles.empty())
		{
			if (m_logger)
			{
				m_logger->logOutputLine("    状态: 无法获取足够的几何数据计算质量重心");
				m_logger->logOutputLine("    注意: 使用简化的几何重心方法");
			}
			return;
		}

		double total_volume = 0.0;
		double centroid_x = 0.0, centroid_y = 0.0, centroid_z = 0.0;

		for (const auto& triangle : triangles)
		{
			if (triangle[0] >= all_vertices.size() ||
				triangle[1] >= all_vertices.size() ||
				triangle[2] >= all_vertices.size())
			{
				continue;
			}

			const auto& v0 = all_vertices[triangle[0]];
			const auto& v1 = all_vertices[triangle[1]];
			const auto& v2 = all_vertices[triangle[2]];

			double edge1[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
			double edge2[3] = { v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2] };

			double normal[3] = {
				edge1[1] * edge2[2] - edge1[2] * edge2[1],
				edge1[2] * edge2[0] - edge1[0] * edge2[2],
				edge1[0] * edge2[1] - edge1[1] * edge2[0]
			};

			double tri_center[3] = {
				(v0[0] + v1[0] + v2[0]) / 3.0,
				(v0[1] + v1[1] + v2[1]) / 3.0,
				(v0[2] + v1[2] + v2[2]) / 3.0
			};

			double vol_contrib = (normal[0] * tri_center[0] +
				normal[1] * tri_center[1] +
				normal[2] * tri_center[2]) / 6.0;

			if (std::abs(vol_contrib) > 1e-15)
			{
				total_volume += vol_contrib;

				centroid_x += vol_contrib * tri_center[0];
				centroid_y += vol_contrib * tri_center[1];
				centroid_z += vol_contrib * tri_center[2];
			}
		}

		if (std::abs(total_volume) > 1e-12)
		{
			centroid_x /= total_volume;
			centroid_y /= total_volume;
			centroid_z /= total_volume;

			if (m_logger)
			{
				m_logger->logOutputLine("    质量重心坐标: (" + std::to_string(centroid_x) + ", " + std::to_string(centroid_y) + ", " + std::to_string(centroid_z) + ")");
				m_logger->logOutputLine("    计算体积: " + std::to_string(std::abs(total_volume)));
				m_logger->logOutputLine("    计算方法: 基于体积积分的真正质量重心");
				m_logger->logOutputLine("    三角形面数: " + std::to_string(triangles.size()));
			}
		}
		else
		{
			if (m_logger)
			{
				m_logger->logOutputLine("    警告: 计算的体积为零或几乎为零");
				m_logger->logOutputLine("    可能原因: 开放几何体、法向量方向不一致或网格问题");
			}
		}
	}
	catch (const std::exception& e)
	{
		if (m_logger)
		{
			m_logger->logOutputLine("    错误: 计算质量重心时发生异常: " + std::string(e.what()));
		}
	}
}

