#include "MeshProcessor.h"
#include <mesh/pfMesh.h>
#include <mesh/pfMeshData.h>
#include <mesh/pfGlobalParameters.h>
#include <mesh/pfLocalSizeParameters.h>
#include <mesh/pfInflationParameters.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <iostream>
#include <functional>
#include <optional>
#include <set>

MeshProcessor::MeshProcessor(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument)
    : m_pfDocument(pfDocument)
    , m_pfMesh(nullptr)
{
}

MeshProcessor::~MeshProcessor()
{
}

bool MeshProcessor::initialize()
{
    if (!m_pfDocument)
    {
        setError("PFDocument为空");
        return false;
    }
    
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = m_pfDocument->getMeshEnvironment();
    m_pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    
    if (!m_pfMesh)
    {
        setError("无法获取PFMesh环境");
        return false;
    }
    
    return true;
}

bool MeshProcessor::setGlobalParameters(const MeshParameters& parameters, const ProjectModelData* modelData)
{
    if (!m_pfMesh)
    {
        setError("PFMesh未初始化");
        return false;
    }
    
    PREPRO_MESH_NAMESPACE::PFGlobalParameters* globalParameters = m_pfMesh->getGlobalParameters();
    if (!globalParameters)
    {
        setError("无法获取全局网格参数");
        return false;
    }
    
    PREPRO_BASE_NAMESPACE::PFStatus status;
    
    status = globalParameters->setSize(parameters.minSize, parameters.maxSize);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置网格尺寸失败");
        return false;
    }
    
    status = globalParameters->setGrowthRate(parameters.growthRate);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置增长率失败");
        return false;
    }
    
    status = globalParameters->setCurvatureNormalAngle(parameters.curvatureNormalAngle);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置曲率法向角度失败");
        return false;
    }
    
    status = globalParameters->setProximityEnabled(parameters.proximityEnabled);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置邻近检测失败");
        return false;
    }
    
    status = globalParameters->setCellsPerGap(parameters.cellsPerGap);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置间隙单元数失败");
        return false;
    }
    
    status = globalParameters->setMinimumGapSize(parameters.minimumGapSize);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置最小间隙尺寸失败");
        return false;
    }
    
    status = globalParameters->setAllTrianglesEnabled(parameters.allTrianglesEnabled);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置全部三角形选项失败");
        return false;
    }
    
    status = globalParameters->setMeshQualityOptimizationEnabled(parameters.meshQualityOptimizationEnabled);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置网格质量优化选项失败");
        return false;
    }
    
    status = globalParameters->setInflationMinimumQuality(parameters.inflationMinimumQuality);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置膨胀最小质量失败");
        return false;
    }
    
    status = globalParameters->setInflationSeparatingAngle(parameters.inflationSeparatingAngle);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置膨胀分离角度失败");
        return false;
    }
    
    status = globalParameters->setInflationMaximumHeightBaseRatio(parameters.inflationMaximumHeightBaseRatio);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("设置膨胀最大高度基比失败");
        return false;
    }
    
    // 如果提供了边界层参数，在设置完全局参数后自动设置边界层
    if (parameters.boundaryLayerParams.has_value() && parameters.fluidZoneSetName.has_value())
    {
        printProgress("全局网格参数设置完成，开始设置边界层参数...");
        if (!setBoundaryLayersByFluidZoneSet(parameters.fluidZoneSetName.value(), 
                                             parameters.boundaryLayerParams.value(), 
                                             modelData,
                                             parameters.excludedBoundaryNames))
        {
            // 边界层设置失败不影响全局参数设置，只记录警告
            printProgress("警告: 边界层参数设置失败: " + getLastError());
            // 不返回false，因为全局参数已经成功设置
        }
        else
        {
            printProgress("边界层参数设置完成");
        }
    }
    
    return true;
}

bool MeshProcessor::setLocalMeshParameters(const std::vector<LocalFluidMeshItem>& localMeshItems,
                                            const ProjectModelData* modelData)
{
    if (!m_pfMesh || !m_pfDocument)
    {
        setError("PFMesh或PFDocument未初始化");
        return false;
    }
    
    if (localMeshItems.empty())
    {
        return true;
    }
    
    if (!modelData)
    {
        setError("设置局部网格参数需要modelData以确定RefinementSet类型");
        return false;
    }
    
    PREPRO_MESH_NAMESPACE::PFLocalSizeParameters* localSizeParams = m_pfMesh->getLocalSizeParameters();
    if (!localSizeParams)
    {
        setError("无法获取局部尺寸参数对象");
        return false;
    }
    
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfGeometryEnvironment = m_pfDocument->getGeometryEnvironment();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry =
        dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfGeometryEnvironment);
    if (!pfGeometry)
    {
        setError("无法获取几何环境");
        return false;
    }
    
    PREPRO_BASE_NAMESPACE::PFData geometryData;
    if (pfGeometry->getAllData(geometryData) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("获取几何数据失败");
        return false;
    }
    
    int volumesNum = geometryData.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = geometryData.getVolumes();
    if (!volumes || volumesNum <= 0)
    {
        setError("未找到体对象");
        return false;
    }
    
    const auto& setList = modelData->GetSetList();
    
    for (const auto& item : localMeshItems)
    {
        if (!item.RefinementSet.has_value() || item.RefinementSet->empty())
        {
            printProgress("跳过局部网格项: RefinementSet为空");
            continue;
        }
        
        const std::string& refinementSet = item.RefinementSet.value();
        
        // 在SetList中查找RefinementSet对应的集合
        const SetItemBase* matchedSet = nullptr;
        for (const auto& setItem : setList)
        {
            if (!setItem) continue;
            if (setItem->SetName.has_value() && setItem->SetName.value() == refinementSet)
            {
                matchedSet = setItem.get();
                break;
            }
        }
        
        if (!matchedSet)
        {
            printProgress("警告: 未在SetList中找到RefinementSet \"" + refinementSet + "\"，跳过");
            continue;
        }
        
        // 单位转换：毫米 -> 米（与FluidMeshInfo一致）
        double minSize = item.MinMeshSize.has_value() ? (item.MinMeshSize.value() / 1000.0) : 0.001;
        double maxSize = item.MaxMeshSize.has_value() ? (item.MaxMeshSize.value() / 1000.0) : 0.01;
        double growthRate = item.GrowthRate.value_or(1.2);
        double normalAngle = item.NormalAngle.value_or(18.0);
        
        std::string setType = matchedSet->SetType.value_or("Solid");
        
        if (setType == "Solid")
        {
            // 体类型：对该体所有面组应用相同的局部网格参数
            std::set<std::string> targetVolumeNames;
            targetVolumeNames.insert(refinementSet);
            std::string oldName = modelData->mapNewVolumeNameToOld(refinementSet);
            if (!oldName.empty() && oldName != refinementSet)
            {
                targetVolumeNames.insert(oldName);
            }
            
            for (int i = 0; i < volumesNum; i++)
            {
                PREPRO_BASE_NAMESPACE::PFVolume* volume = volumes[i];
                if (!volume) continue;
                
                char* volName = volume->getName();
                std::string volNameStr = volName ? std::string(volName) : "";
                if (volNameStr.empty() || targetVolumeNames.find(volNameStr) == targetVolumeNames.end())
                {
                    continue;
                }
                
                int groupsNum = volume->getGroupSize();
                PREPRO_BASE_NAMESPACE::PFGroup** groups = volume->getGroups();
                if (!groups || groupsNum <= 0) continue;
                
                for (int j = 0; j < groupsNum; j++)
                {
                    PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
                    if (!group) continue;
                    
                    char* groupName = group->getName();
                    if (!groupName) continue;
                    
                    std::string groupNameStr(groupName);
                    if (groupNameStr.find("node") != std::string::npos ||
                        groupNameStr.find("edge") != std::string::npos ||
                        (groupNameStr.size() > 0 && groupNameStr[0] == '.'))
                    {
                        continue;
                    }
                    
                    PREPRO_BASE_NAMESPACE::PFStatus status = localSizeParams->setMinimunAndMaximunSize(groupNameStr.c_str(), minSize, maxSize);
                    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                    {
                        printProgress("警告: 无法为面组 " + groupNameStr + " 设置局部尺寸");
                        continue;
                    }
                    status = localSizeParams->setGrowthRate(groupNameStr.c_str(), growthRate);
                    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                    {
                        printProgress("警告: 无法为面组 " + groupNameStr + " 设置增长率");
                    }
                    status = localSizeParams->setCurvatureNormalAngle(groupNameStr.c_str(), normalAngle);
                    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
                    {
                        printProgress("警告: 无法为面组 " + groupNameStr + " 设置曲率法向角");
                    }
                    
                    printProgress("已为体 " + volNameStr + " 的面组 " + groupNameStr + " 设置局部网格: min=" +
                        std::to_string(minSize) + ", max=" + std::to_string(maxSize));
                }
            }
        }
        else if (setType == "Face")
        {
            // 面类型：仅对该面组应用局部网格参数
            std::string groupNameStr = refinementSet;
            
            PREPRO_BASE_NAMESPACE::PFStatus status = localSizeParams->setMinimunAndMaximunSize(groupNameStr.c_str(), minSize, maxSize);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为面组 " + groupNameStr + " 设置局部尺寸");
                continue;
            }
            status = localSizeParams->setGrowthRate(groupNameStr.c_str(), growthRate);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为面组 " + groupNameStr + " 设置增长率");
            }
            status = localSizeParams->setCurvatureNormalAngle(groupNameStr.c_str(), normalAngle);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为面组 " + groupNameStr + " 设置曲率法向角");
            }
            
            printProgress("已为面组 " + groupNameStr + " 设置局部网格: min=" +
                std::to_string(minSize) + ", max=" + std::to_string(maxSize));
        }
        else
        {
            printProgress("警告: 不支持的SetType \"" + setType + "\"，跳过");
        }
    }
    
    printProgress("局部网格参数设置完成");
    return true;
}

bool MeshProcessor::createSurfaceMesh()
{
    if (!m_pfMesh)
    {
        setError("PFMesh未初始化");
        return false;
    }
    
    printProgress("正在生成表面网格...");
    
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfMesh->createSurfaceMesh();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("生成表面网格失败");
        return false;
    }
    
    printProgress("表面网格生成成功");
    return true;
}

bool MeshProcessor::createVolumeMeshByGeometry(bool withInflation, bool withPolyhedron)
{
    if (!m_pfMesh)
    {
        setError("PFMesh未初始化");
        return false;
    }
    
    printProgress("正在根据几何生成体积网格...");
    
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfMesh->createVolumeMeshByGeometry(withInflation, withPolyhedron);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("根据几何生成体积网格失败");
        return false;
    }
    
    printProgress("体积网格生成成功");
    return true;
}

bool MeshProcessor::createVolumeMeshBySurfaceMesh(bool withInflation, bool withPolyhedron)
{
    if (!m_pfMesh)
    {
        setError("PFMesh未初始化");
        return false;
    }
    
    printProgress("正在根据表面网格生成体积网格...");
    
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfMesh->createVolumeMeshBySurfaceMesh(withInflation, withPolyhedron);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("根据表面网格生成体积网格失败");
        return false;
    }
    
    printProgress("体积网格生成成功");
    return true;
}

void MeshProcessor::setProgressCallback(std::function<void(const std::string&)> callback)
{
    m_progressCallback = callback;
}

void MeshProcessor::printProgress(const std::string& message)
{
    if (m_progressCallback)
    {
        m_progressCallback(message);
    }
    else
    {
        std::cout << message << std::endl;
    }
}

void MeshProcessor::setError(const std::string& error)
{
    m_lastError = error;
    std::cerr << "MeshProcessor错误: " << error << std::endl;
}

bool MeshProcessor::hasMeshData() const
{
    if (!m_pfMesh)
    {
        return false;
    }
    
    try
    {
        PREPRO_BASE_NAMESPACE::PFData pfData;
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfMesh->getAllData(pfData);
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            return false;
        }
        
        // 检查是否有体积网格数据
        unsigned int volumeSize = pfData.getVolumeSize();
        return volumeSize > 0;
    }
    catch (...)
    {
        return false;
    }
}

bool MeshProcessor::getMeshData(PREPRO_BASE_NAMESPACE::PFData& data) const
{
    if (!m_pfMesh)
    {
        // const 方法中不能调用 setError，错误信息由调用者处理
        return false;
    }

    try
    {
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfMesh->getAllData(data);
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            // const 方法中不能调用 setError，错误信息由调用者处理
            return false;
        }
        
        // 检查是否有网格数据
        if (data.getGroupSize() == 0 && data.getVolumeSize() == 0)
        {
            // const 方法中不能调用 setError，错误信息由调用者处理
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        // const 方法中不能调用 setError，错误信息由调用者处理
        return false;
    }
    catch (...)
    {
        // const 方法中不能调用 setError，错误信息由调用者处理
        return false;
    }
}

bool MeshProcessor::setBoundaryLayersByFluidZoneSet(const std::string& fluidZoneSetName, 
                                                     const BoundaryLayerParameters& parameters,
                                                     const ProjectModelData* modelData,
                                                     const std::set<std::string>& excludedBoundaryNames)
{
    if (!m_pfMesh)
    {
        setError("PFMesh未初始化");
        return false;
    }
    
    if (!m_pfDocument)
    {
        setError("PFDocument为空");
        return false;
    }
    
    // 检查是否启用边界层
    if (!parameters.isBoundaryLayers)
    {
        printProgress("边界层未启用，跳过设置");
        return true;
    }
    
    printProgress("开始设置边界层参数，流体区域集合: " + fluidZoneSetName);
    
    // 获取几何环境
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfGeometryEnvironment = m_pfDocument->getGeometryEnvironment();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = 
        dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfGeometryEnvironment);
    if (!pfGeometry)
    {
        setError("无法获取几何环境");
        return false;
    }
    
    // 获取几何数据
    PREPRO_BASE_NAMESPACE::PFData geometryData;
    PREPRO_BASE_NAMESPACE::PFStatus dataStatus = pfGeometry->getAllData(geometryData);
    if (dataStatus != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("获取几何数据失败，状态码: " + std::to_string(static_cast<int>(dataStatus)));
        return false;
    }
    
    printProgress("调试: 成功获取几何数据");
    
    // 获取所有体对象
    int volumesNum = geometryData.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = geometryData.getVolumes();
    
    // 输出所有体对象的当前名称（用于调试，确认重命名是否生效）
    if (volumes && volumesNum > 0)
    {
        printProgress("调试: 当前几何环境中的体对象名称:");
        for (int i = 0; i < volumesNum; i++)
        {
            if (volumes[i])
            {
                char* name = volumes[i]->getName();
                unsigned int id = volumes[i]->getId();
                printProgress("  [" + std::to_string(i) + "] ID=" + std::to_string(id) + 
                             ", Name=\"" + (name ? std::string(name) : "未命名") + "\"");
            }
        }
    }
    
    printProgress("调试: 获取体对象数量: " + std::to_string(volumesNum));
    printProgress("调试: volumes指针: " + std::string(volumes ? "非空" : "空"));
    
    if (!volumes || volumesNum <= 0)
    {
        setError("未找到体对象 (volumesNum=" + std::to_string(volumesNum) + ", volumes=" + 
                 std::string(volumes ? "非空" : "空") + ")");
        return false;
    }
    
    printProgress("调试: 找到 " + std::to_string(volumesNum) + " 个体对象，开始遍历");
    
    // 获取边界层参数对象
    PREPRO_MESH_NAMESPACE::PFInflationParameters* inflationParameters = m_pfMesh->getInflationParameters();
    if (!inflationParameters)
    {
        setError("无法获取边界层参数对象");
        return false;
    }
    
    // 如果提供了modelData，从SetList中获取匹配的体对象名称
    // 注意：SetList中的ID可能与导入后的体对象ID不匹配，所以使用名称匹配
    std::set<std::string> targetVolumeNames;
    if (modelData)
    {
        const auto& setList = modelData->GetSetList();
        for (const auto& setItem : setList)
        {
            if (!setItem) continue;
            
            // 检查SetName是否匹配fluidZoneSetName
            if (setItem->SetName.has_value() && setItem->SetName.value() == fluidZoneSetName)
            {
                // 检查是否为Solid类型
                if (setItem->SetType.has_value() && setItem->SetType.value() == "Solid")
                {
                    auto* solidSet = dynamic_cast<SetSolidItem*>(setItem.get());
                    if (solidSet)
                    {
                        // 1) 新名称（SetName），例如 "LT"
                        targetVolumeNames.insert(fluidZoneSetName);
                        
                        // 2) 尝试通过ProjectModelData的重命名映射，将新名称映射回原始名称
                        std::string oldName = modelData->mapNewVolumeNameToOld(fluidZoneSetName);
                        if (!oldName.empty() && oldName != fluidZoneSetName)
                        {
                            targetVolumeNames.insert(oldName);
                            printProgress("从重命名映射中找到体对象名称映射: \"" + fluidZoneSetName + "\" -> \"" + oldName + "\"");
                        }
                        
                        printProgress("从SetList中找到集合 \"" + fluidZoneSetName + 
                                     "\"，将使用名称匹配体对象");
                    }
                }
                break;
            }
        }
    }
    
    // 遍历所有体对象，查找匹配的体对象
    bool foundAnyVolume = false;
    
    // 输出调试信息：显示所有体对象的ID和名称
    printProgress("几何环境中共有 " + std::to_string(volumesNum) + " 个体对象");
    if (!targetVolumeNames.empty())
    {
        std::string targetNamesStr = "目标体对象名称: ";
        for (const auto& name : targetVolumeNames)
        {
            targetNamesStr += "\"" + name + "\" ";
        }
        printProgress(targetNamesStr);
    }
    
    for (int i = 0; i < volumesNum; i++)
    {
        PREPRO_BASE_NAMESPACE::PFVolume* volume = volumes[i];
        if (!volume) continue;
        
        unsigned int volumeId = volume->getId();
        char* volumeName = volume->getName();
        std::string volNameStr = volumeName ? std::string(volumeName) : "未命名";
        
        printProgress("检查体对象 [" + std::to_string(i) + "]: ID=" + std::to_string(volumeId) + ", Name=\"" + volNameStr + "\"");
        
        // 如果提供了modelData且有目标名称集合，检查是否匹配
        if (!targetVolumeNames.empty())
        {
            if (targetVolumeNames.find(volNameStr) == targetVolumeNames.end())
            {
                printProgress("  体对象名称 \"" + volNameStr + "\" 不在目标名称集合中，跳过");
                continue; // 跳过不匹配的体对象
            }
            else
            {
                printProgress("  体对象名称 \"" + volNameStr + "\" 匹配目标名称集合");
            }
        }
        
        if (!volumeName) 
        {
            printProgress("  警告: 体对象ID " + std::to_string(volumeId) + " 没有名称，跳过");
            continue;
        }
        
        // 获取该体对象的所有组
        int groupsNum = volume->getGroupSize();
        PREPRO_BASE_NAMESPACE::PFGroup** groups = volume->getGroups();
        
        printProgress("  体对象 " + volNameStr + " 有 " + std::to_string(groupsNum) + " 个组");
        
        if (!groups || groupsNum <= 0) 
        {
            printProgress("  警告: 体对象 " + volNameStr + " 没有组，跳过");
            continue;
        }
        
        // 对每个组设置边界层参数
        int validGroupCount = 0;
        for (int j = 0; j < groupsNum; j++)
        {
            PREPRO_BASE_NAMESPACE::PFGroup* group = groups[j];
            if (!group) 
            {
                printProgress("    组 [" + std::to_string(j) + "] 为空，跳过");
                continue;
            }
            
            char* groupName = group->getName();
            if (!groupName) 
            {
                printProgress("    组 [" + std::to_string(j) + "] 没有名称，跳过");
                continue;
            }
            
            std::string groupNameStr(groupName);
            printProgress("    处理组 [" + std::to_string(j) + "]: " + groupNameStr);
            
            // 跳过系统组（如.nodes, .edges等）
            if (groupNameStr.find("node") != std::string::npos || 
                groupNameStr.find("edge") != std::string::npos ||
                groupNameStr[0] == '.')
            {
                printProgress("      跳过系统组: " + groupNameStr);
                continue;
            }
            
            // 跳过非Wall类型的边界条件对应的面组（如VelocityInlet、PressureOutlet等）
            if (!excludedBoundaryNames.empty() && excludedBoundaryNames.find(groupNameStr) != excludedBoundaryNames.end())
            {
                printProgress("      跳过非Wall类型边界条件对应的面组: " + groupNameStr + " (边界类型不是Wall)");
                continue;
            }
            
            validGroupCount++;
            
            // 设置边界层参数
            PREPRO_BASE_NAMESPACE::PFStatus status;
            
            // 首先开启边界层
            status = inflationParameters->setInflationOn(volNameStr.c_str(), groupNameStr.c_str(), true);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为体对象 " + volNameStr + " 的组 " + groupNameStr + " 开启边界层");
                continue;
            }
            
            // 设置第一层高度
            status = inflationParameters->setInitialHeight(volNameStr.c_str(), groupNameStr.c_str(), 
                                                          parameters.firstLayerHeight);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为体对象 " + volNameStr + " 的组 " + groupNameStr + " 设置第一层高度");
                continue;
            }
            
            // 设置层数
            status = inflationParameters->setLayerNumber(volNameStr.c_str(), groupNameStr.c_str(), 
                                                         parameters.layersNumber);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为体对象 " + volNameStr + " 的组 " + groupNameStr + " 设置层数");
                continue;
            }
            
            // 设置高度比（增长率）
            status = inflationParameters->setHeightRatio(volNameStr.c_str(), groupNameStr.c_str(), 
                                                         parameters.growthRate);
            if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                printProgress("警告: 无法为体对象 " + volNameStr + " 的组 " + groupNameStr + " 设置高度比");
                continue;
            }
            
            foundAnyVolume = true;
            printProgress("成功为体对象 " + volNameStr + " 的组 " + groupNameStr + 
                         " 设置边界层: 高度=" + std::to_string(parameters.firstLayerHeight) +
                         ", 层数=" + std::to_string(parameters.layersNumber) +
                         ", 增长率=" + std::to_string(parameters.growthRate));
        }
    }
    
    if (!foundAnyVolume)
    {
        setError("未找到匹配的体对象或组来设置边界层");
        return false;
    }
    
    printProgress("边界层参数设置完成");
    return true;
}
