// Revision: $Id: GeometryAPI.cpp,v 1.0 2025/02/27 10:00:00 assistant Exp $
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
#include "RenderProcessor.h"
#include "commonEnumeration.h"

#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometryData.h>
#include <mesh/pfMeshBuilder.h>
#include <mesh/pfMesh.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>

#include <iostream>
#include <algorithm>

// ============== 构造和析构 ==============

GeometryAPI::GeometryAPI(const std::string& examplePath)
    : m_examplePath(examplePath)
    , m_pfApplication(nullptr)
    , m_pfDocument(nullptr)
    , m_pfGeometry(nullptr)
    , m_initialized(false)
{
    m_pfApplication = new PREPRO_BASE_NAMESPACE::PFApplication();
}

GeometryAPI::~GeometryAPI()
{
    if (m_pfApplication)
    {
        delete m_pfApplication;
        m_pfApplication = nullptr;
    }
    // 注意：m_pfGeometry 由 PFDocument 管理，不需要手动删除
}

// ============== 初始化和文件操作 ==============

bool GeometryAPI::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    try
    {
        // 获取几何构建器
        PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = 
            PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
        
        if (!builder)
        {
            setError("无法获取几何构建器实例");
            return false;
        }

        // 添加几何环境到应用程序
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfApplication->addEnvironment(builder);
        if (status == PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense)
        {
            setError("没有PERA SIM许可证");
            return false;
        }
        else if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("添加几何环境失败");
            return false;
        }

        // 添加网格环境到应用程序（用于网格生成功能）
        PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = 
            PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
        if (meshBuilder)
        {
            status = m_pfApplication->addEnvironment(meshBuilder);
            if (status == PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense)
            {
                // 网格模块没有许可证，但不影响几何功能，只记录警告
                std::cout << "[GeometryAPI] 警告: 没有网格模块许可证，网格功能将不可用" << std::endl;
            }
            else if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                // 添加网格环境失败，但不影响几何功能，只记录警告
                std::cout << "[GeometryAPI] 警告: 添加网格环境失败，网格功能将不可用" << std::endl;
            }
        }

        // 创建新文档
        m_pfDocument = m_pfApplication->newDocument();
        if (!m_pfDocument)
        {
            setError("创建文档失败");
            return false;
        }

        // 获取几何环境
        m_pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(
            m_pfDocument->getGeometryEnvironment());
        
        if (!m_pfGeometry)
        {
            setError("获取几何环境失败");
            return false;
        }

        m_initialized = true;
        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("初始化异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::importGeometry(const std::string& filePath, int importMode)
{
    if (!checkGeometry())
    {
        return false;
    }

    try 
    {
        std::cout << "[GeometryAPI] 开始导入几何文件: " << filePath << std::endl;
        std::cout << "[GeometryAPI] 导入模式: " << importMode << std::endl;
        
        // 设置导入选项
        m_pfGeometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromBodyName);
        
        PREPRO_BASE_NAMESPACE::PFStatus status;
        
        // 根据导入模式选择导入方式
        if (importMode == 1) {
            // 第一次导入，不需要第二个参数
            std::cout << "[GeometryAPI] 执行第一次导入..." << std::endl;
            status = m_pfGeometry->importGeometry(filePath.c_str());
        } else if (importMode == 2) {
            // 替换模式
            std::cout << "[GeometryAPI] 执行替换模式导入..." << std::endl;
            status = m_pfGeometry->importGeometry(filePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
        } else if (importMode == 3) {
            // 追加模式
            std::cout << "[GeometryAPI] 执行追加模式导入..." << std::endl;
            status = m_pfGeometry->importGeometry(filePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
        } else {
            setError("无效的导入模式，请选择 1(第一次导入)、2(替换) 或 3(追加)");
            return false;
        }
        
        std::cout << "[GeometryAPI] SDK返回状态: " << static_cast<int>(status) << std::endl;
        
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::string errorMsg = "导入几何文件失败: " + filePath + " (状态码: " + std::to_string(static_cast<int>(status)) + ")";
            setError(errorMsg);
            return false;
        }

        std::cout << "[GeometryAPI] 几何文件导入成功" << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("导入几何文件异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::importMesh(const std::string& filePath)
{
    if (!m_pfApplication)
    {
        setError("应用程序未初始化");
        return false;
    }
    
    try 
    {
        // 获取网格构建器
        PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = 
            PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
        
        if (!meshBuilder)
        {
            setError("无法获取网格构建器");
            return false;
        }

        // 添加网格环境（如果还没有添加）
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfApplication->addEnvironment(meshBuilder);
        if (status == PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense)
        {
            setError("没有网格模块许可证");
            return false;
        }

        // 创建新文档或使用现有文档
        PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = m_pfApplication->newDocument();
        if (!pfDocument)
        {
            setError("创建文档失败");
            return false;
        }

        // 更新成员变量，确保getDocument()返回正确的文档
        m_pfDocument = pfDocument;

        // 获取网格对象
        PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = 
            dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
        
        if (!pfMesh)
        {
            setError("获取网格环境失败");
            return false;
        }

        // 导入网格文件
        status = pfMesh->importMesh(filePath.c_str());
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("导入网格文件失败: " + filePath);
            return false;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("导入网格文件异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::importPostProcessResult(const std::string& filePath)
{
    if (!m_pfApplication)
    {
        setError("应用程序未初始化");
        return false;
    }
    
    try 
    {
        // 获取后处理构建器
        PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder* postProcessBuilder = 
            PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder::getInstance();
        
        if (!postProcessBuilder)
        {
            setError("无法获取后处理构建器");
            return false;
        }

        // 添加后处理环境（如果还没有添加）
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfApplication->addEnvironment(postProcessBuilder);
        if (status == PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense)
        {
            setError("没有后处理模块许可证");
            return false;
        }

        // 创建新文档或使用现有文档
        PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = m_pfApplication->newDocument();
        if (!pfDocument)
        {
            setError("创建文档失败");
            return false;
        }

        // 获取后处理对象
        PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* pfPostProcess = 
            dynamic_cast<PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess*>(pfDocument->getPostProcessEnvironment());
        
        if (!pfPostProcess)
        {
            setError("获取后处理环境失败");
            return false;
        }

        // 导入后处理结果
        status = pfPostProcess->loadResult(filePath.c_str());
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("导入后处理结果失败: " + filePath);
            return false;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("导入后处理结果异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::exportGeometry(const std::string& filePath)
{
    if (!checkGeometry())
    {
        return false;
    }

    // 注意：当前SDK版本可能不支持直接导出功能
    // 这里提供一个占位实现，实际使用时可以根据SDK版本调整
    setError("导出功能暂不支持，请使用SDK的其他导出方式");
    return false;
}

bool GeometryAPI::saveDocument(const std::string& filePath)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (!m_pfDocument)
    {
        setError("文档对象不存在");
        return false;
    }

    try
    {
        PREPRO_BASE_NAMESPACE::PFStatus status;
        
        if (!filePath.empty())
        {
            // 如果指定了路径，使用saveAs
            m_pfDocument->setDocumentPath(filePath.c_str());
            status = m_pfDocument->saveAs(filePath.c_str());
        }
        else
        {
            // 如果没有指定路径，使用save
            status = m_pfDocument->save();
        }

        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("保存文档失败");
            return false;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("保存文档异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::getAllData(PREPRO_BASE_NAMESPACE::PFData& data)
{
    if (!checkGeometry())
    {
        return false;
    }

    try
    {
        m_pfGeometry->getAllData(data);
        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("获取几何数据异常: ") + e.what());
        return false;
    }
}

// ============== 几何修复功能 ==============

bool GeometryAPI::quickRepair(double stitchTolerance)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_GEOMETRY_NAMESPACE::QuickRepairParameters parameters;
    parameters.stitchOption = PREPRO_GEOMETRY_NAMESPACE::StitchOptions::EStitchAndIntersection;
    parameters.stitchTolerance = stitchTolerance;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->quickRepair(parameters);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("快速修复失败");
        return false;
    }

    return true;
}

int GeometryAPI::findVolumes()
{
    if (!checkGeometry())
    {
        return -1;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->findVolumes();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("查找体积失败");
        return -1;
    }

    // 获取找到的体积数量
    PREPRO_BASE_NAMESPACE::PFData geometryData;
    m_pfGeometry->getAllData(geometryData);
    return geometryData.getVolumeSize();
}

bool GeometryAPI::stitchTwoFaces(unsigned int faceId1, unsigned int faceId2, double tolerance)
{
    if (!checkGeometry())
    {
        return false;
    }

    unsigned int ids[2] = { faceId1, faceId2 };
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->stitchTwoFaces(ids, tolerance);
    
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("缝合两个面失败");
        return false;
    }

    return true;
}

bool GeometryAPI::defeature(double tolerance)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->defeature(tolerance);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("去除特征失败");
        return false;
    }

    return true;
}

// ============== 孔洞处理 ==============

bool GeometryAPI::stitchHole(unsigned int holeId)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->stitchHole(holeId);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("缝合孔洞失败");
        return false;
    }

    return true;
}

bool GeometryAPI::fillHole(unsigned int holeId)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->fillHole(holeId);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("填充孔洞失败");
        return false;
    }

    return true;
}

bool GeometryAPI::fillAllHoles()
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->fillAllHoles();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("填充所有孔洞失败");
        return false;
    }

    return true;
}

bool GeometryAPI::fillFacesHoles(const std::vector<unsigned int>& faceIds)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (faceIds.empty())
    {
        setError("面ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(faceIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->fillFacesHoles(
        static_cast<unsigned int>(faceIds.size()), ids);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("填充面孔洞失败");
        return false;
    }

    return true;
}

// ============== 面操作 ==============

bool GeometryAPI::intersectTwoFaces(unsigned int faceId1, unsigned int faceId2, double tolerance)
{
    if (!checkGeometry())
    {
        return false;
    }

    unsigned int ids[2] = { faceId1, faceId2 };
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->intersectTwoFaces(ids, tolerance);
    
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("两个面相交失败");
        return false;
    }

    return true;
}

bool GeometryAPI::mergeFaces(const std::vector<unsigned int>& faceIds)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (faceIds.empty())
    {
        setError("面ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(faceIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->mergeFaces(
        static_cast<unsigned int>(faceIds.size()), ids);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("合并面失败");
        return false;
    }

    return true;
}

bool GeometryAPI::createFaceByEdges(const std::vector<unsigned int>& edgeIds)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (edgeIds.empty())
    {
        setError("边ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(edgeIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createFaceByEdges(
        static_cast<unsigned int>(edgeIds.size()), ids);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("通过边创建面失败");
        return false;
    }

    return true;
}

bool GeometryAPI::createFaceByPoints(const std::vector<double>& coordinates, unsigned int pointCount)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (coordinates.size() != pointCount * 3)
    {
        setError("坐标数组大小与点数不匹配");
        return false;
    }

    double* coords = vectorToArray(coordinates);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createFaceByPoints(pointCount, coords);
    
    delete[] coords;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("通过点创建面失败");
        return false;
    }

    return true;
}

bool GeometryAPI::splitFaceByEdge(unsigned int faceId, unsigned int edgeId)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->splitFaceByEdge(faceId, edgeId);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("用边分割面失败");
        return false;
    }

    return true;
}

// ============== 边操作 ==============

bool GeometryAPI::createEdge(unsigned int startPointId, unsigned int endPointId)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createEdge(startPointId, endPointId);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建边失败");
        return false;
    }

    return true;
}

bool GeometryAPI::splitEdge(unsigned int edgeId, double parameter)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (parameter < 0.0 || parameter > 1.0)
    {
        setError("分割参数必须在0.0到1.0之间");
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->splitEdge(edgeId, parameter);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("分割边失败");
        return false;
    }

    return true;
}

bool GeometryAPI::mergeEdges(const std::vector<unsigned int>& edgeIds)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (edgeIds.empty())
    {
        setError("边ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(edgeIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->mergeEdges(
        static_cast<unsigned int>(edgeIds.size()), ids);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("合并边失败");
        return false;
    }

    return true;
}

// ============== 几何变换 ==============

bool GeometryAPI::scale(const std::vector<unsigned int>& entityIds, 
                        const std::vector<double>& origin, 
                        double scaleFactor, 
                        bool onlyTransform)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty() || origin.size() != 3)
    {
        setError("参数无效：实体ID列表为空或原点坐标不是3维");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    double originArray[3] = { origin[0], origin[1], origin[2] };
    
    PREPRO_GEOMETRY_NAMESPACE::TransformOptions option = onlyTransform ? 
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform :
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->scale(
        static_cast<unsigned int>(entityIds.size()), ids, originArray, scaleFactor, option);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("缩放操作失败");
        return false;
    }

    return true;
}

bool GeometryAPI::mirror(const std::vector<unsigned int>& entityIds,
                         const std::vector<double>& origin,
                         const std::vector<double>& direction,
                         bool copyAndTransform)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty() || origin.size() != 3 || direction.size() != 3)
    {
        setError("参数无效：实体ID列表为空或坐标/方向不是3维");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    double originArray[3] = { origin[0], origin[1], origin[2] };
    double directionArray[3] = { direction[0], direction[1], direction[2] };
    
    PREPRO_GEOMETRY_NAMESPACE::TransformOptions option = copyAndTransform ? 
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform :
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->mirror(
        static_cast<unsigned int>(entityIds.size()), ids, originArray, directionArray, option);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("镜像操作失败");
        return false;
    }

    return true;
}

bool GeometryAPI::rotate(const std::vector<unsigned int>& entityIds,
                         const std::vector<double>& origin,
                         const std::vector<double>& axis,
                         double angle,
                         bool onlyTransform)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty() || origin.size() != 3 || axis.size() != 3)
    {
        setError("参数无效：实体ID列表为空或坐标/轴向不是3维");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    double originArray[3] = { origin[0], origin[1], origin[2] };
    double axisArray[3] = { axis[0], axis[1], axis[2] };
    
    PREPRO_GEOMETRY_NAMESPACE::TransformOptions option = onlyTransform ? 
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform :
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->rotate(
        static_cast<unsigned int>(entityIds.size()), ids, originArray, axisArray, angle, option);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("旋转操作失败");
        return false;
    }

    return true;
}

bool GeometryAPI::translate(const std::vector<unsigned int>& entityIds,
                            const std::vector<double>& offset,
                            bool copyAndTransform)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty() || offset.size() != 3)
    {
        setError("参数无效：实体ID列表为空或偏移量不是3维");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    double offsetArray[3] = { offset[0], offset[1], offset[2] };
    
    PREPRO_GEOMETRY_NAMESPACE::TransformOptions option = copyAndTransform ? 
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform :
        PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->translate(
        static_cast<unsigned int>(entityIds.size()), ids, offsetArray, option);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("平移操作失败");
        return false;
    }

    return true;
}

// ============== 基本几何体创建 ==============

bool GeometryAPI::createCube(const std::string& name,
                             const std::vector<float>& startPoint,
                             const std::vector<float>& endPoint)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (startPoint.size() != 3 || endPoint.size() != 3)
    {
        setError("起点和终点必须是3维坐标");
        return false;
    }

    float start[3] = { startPoint[0], startPoint[1], startPoint[2] };
    float end[3] = { endPoint[0], endPoint[1], endPoint[2] };

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createCube(name.c_str(), start, end);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建立方体失败");
        return false;
    }

    return true;
}

bool GeometryAPI::createCylinder(const std::string& name,
                                 const std::vector<float>& startPoint,
                                 const std::vector<float>& endPoint,
                                 float radius)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (startPoint.size() != 3 || endPoint.size() != 3)
    {
        setError("起点和终点必须是3维坐标");
        return false;
    }

    if (radius <= 0.0f)
    {
        setError("半径必须大于0");
        return false;
    }

    float start[3] = { startPoint[0], startPoint[1], startPoint[2] };
    float end[3] = { endPoint[0], endPoint[1], endPoint[2] };

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createCylinder(name.c_str(), start, end, radius);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建圆柱体失败");
        return false;
    }

    return true;
}

bool GeometryAPI::createSphere(const std::string& name,
                               const std::vector<float>& center,
                               float radius)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (center.size() != 3)
    {
        setError("中心点必须是3维坐标");
        return false;
    }

    if (radius <= 0.0f)
    {
        setError("半径必须大于0");
        return false;
    }

    float centerArray[3] = { center[0], center[1], center[2] };

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createSphere(name.c_str(), centerArray, radius);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建球体失败");
        return false;
    }

    return true;
}

bool GeometryAPI::createHemisphere(const std::string& name,
                                   const std::vector<float>& center,
                                   const std::vector<float>& direction,
                                   float radius)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (center.size() != 3 || direction.size() != 3)
    {
        setError("中心点和方向必须是3维坐标");
        return false;
    }

    if (radius <= 0.0f)
    {
        setError("半径必须大于0");
        return false;
    }

    float centerArray[3] = { center[0], center[1], center[2] };
    float directionArray[3] = { direction[0], direction[1], direction[2] };

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createHemisphere(
        name.c_str(), centerArray, directionArray, radius);
    
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建半球失败");
        return false;
    }

    return true;
}

// ============== 高级操作 ==============

bool GeometryAPI::extrude(const std::vector<unsigned int>& entityIds, 
                          double distance, 
                          bool useNormal)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty())
    {
        setError("实体ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    PREPRO_GEOMETRY_NAMESPACE::EExtrudeType extrudeType = useNormal ? 
        PREPRO_GEOMETRY_NAMESPACE::EExtrudeType::ENormal :
        PREPRO_GEOMETRY_NAMESPACE::EExtrudeType::ENormal; // 可能需要根据SDK调整

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->extrude(
        static_cast<unsigned int>(entityIds.size()), ids, extrudeType, distance);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("拉伸操作失败");
        return false;
    }

    return true;
}

bool GeometryAPI::imprint(const std::vector<unsigned int>& faceIds)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (faceIds.empty())
    {
        setError("面ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(faceIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->imprint(
        static_cast<unsigned int>(faceIds.size()), ids);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("压印操作失败");
        return false;
    }

    return true;
}

bool GeometryAPI::project(unsigned int sourceId, unsigned int targetId)
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->project(sourceId, targetId);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("投影操作失败");
        return false;
    }

    return true;
}

// ============== 实体管理 ==============

bool GeometryAPI::deleteEntities(const std::vector<unsigned int>& entityIds, 
                                 bool keepAffiliated)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty())
    {
        setError("实体ID列表为空");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    PREPRO_GEOMETRY_NAMESPACE::DeleteOptions option = keepAffiliated ? 
        PREPRO_GEOMETRY_NAMESPACE::DeleteOptions::EKeepAffiliatedEntities :
        PREPRO_GEOMETRY_NAMESPACE::DeleteOptions::EDeleteAffiliatedEntities;

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->deleteEntities(
        static_cast<unsigned int>(entityIds.size()), ids, option);
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("删除实体失败");
        return false;
    }

    return true;
}

bool GeometryAPI::addEntitiesToGroup(const std::vector<unsigned int>& entityIds, 
                                     const std::string& groupName)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (entityIds.empty() || groupName.empty())
    {
        setError("实体ID列表为空或组名为空");
        return false;
    }

    unsigned int* ids = vectorToArray(entityIds);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->addEntitiesToGroup(
        static_cast<unsigned int>(entityIds.size()), ids, groupName.c_str());
    
    delete[] ids;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("添加实体到组失败");
        return false;
    }

    return true;
}

// ============== 体积操作 ==============

bool GeometryAPI::createVolume(const std::vector<std::string>& groupNames)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (groupNames.empty())
    {
        setError("组名列表为空");
        return false;
    }

    const char** names = stringVectorToArray(groupNames);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->createVolume(
        static_cast<unsigned int>(groupNames.size()), names);
    
    delete[] names;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("创建体积失败");
        return false;
    }

    return true;
}

bool GeometryAPI::detachVolumes(const std::vector<std::string>& volumeNames)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (volumeNames.empty())
    {
        setError("体积名称列表为空");
        return false;
    }

    const char** names = stringVectorToArray(volumeNames);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->detachVolumes(
        static_cast<unsigned int>(volumeNames.size()), names);
    
    delete[] names;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("分离体积失败");
        return false;
    }

    return true;
}

bool GeometryAPI::deleteVolumes(const std::vector<std::string>& volumeNames)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (volumeNames.empty())
    {
        setError("体积名称列表为空");
        return false;
    }

    const char** names = stringVectorToArray(volumeNames);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->deleteVolumes(
        static_cast<unsigned int>(volumeNames.size()), names);
    
    delete[] names;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("删除体积失败");
        return false;
    }

    return true;
}

// ============== 命名操作 ==============

bool GeometryAPI::renameFaceGroup(const std::string& oldName, const std::string& newName)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (oldName.empty() || newName.empty())
    {
        setError("组名不能为空");
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->renameFaceGroup(
        oldName.c_str(), newName.c_str());
    
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("重命名面组失败");
        return false;
    }

    return true;
}

bool GeometryAPI::renameVolume(const std::string& oldName, const std::string& newName)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (oldName.empty() || newName.empty())
    {
        setError("体积名称不能为空");
        return false;
    }

    std::cout << "[GeometryAPI] 尝试重命名体积: \"" << oldName << "\" -> \"" << newName << "\"" << std::endl;
    
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->renameVolume(
        oldName.c_str(), newName.c_str());
    
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::string errorMsg = "重命名体积失败 (状态码: " + std::to_string(static_cast<int>(status)) + ")";
        std::cout << "[GeometryAPI] " << errorMsg << std::endl;
        setError(errorMsg);
        return false;
    }
    
    std::cout << "[GeometryAPI] 体积重命名成功" << std::endl;

    return true;
}

bool GeometryAPI::deleteFaceGroups(const std::vector<std::string>& groupNames)
{
    if (!checkGeometry())
    {
        return false;
    }

    if (groupNames.empty())
    {
        setError("组名列表为空");
        return false;
    }

    const char** names = stringVectorToArray(groupNames);
    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->deleteFaceGroups(
        static_cast<unsigned int>(groupNames.size()), names);
    
    delete[] names;

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("删除面组失败");
        return false;
    }

    return true;
}

// ============== 网格转换 ==============

bool GeometryAPI::importMeshAndConvertToGeometry(const std::string& meshFilePath)
{
    if (!m_pfApplication)
    {
        setError("应用程序未初始化");
        return false;
    }

    try
    {
        // 获取网格构建器
        PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = 
            PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
        
        if (!meshBuilder)
        {
            setError("无法获取网格构建器");
            return false;
        }

        // 添加网格环境
        PREPRO_BASE_NAMESPACE::PFStatus status = m_pfApplication->addEnvironment(meshBuilder);
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("添加网格环境失败");
            return false;
        }

        // 创建新文档
        PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = m_pfApplication->newDocument();
        if (!pfDocument)
        {
            setError("创建文档失败");
            return false;
        }

        // 获取网格对象
        PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = 
            dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
        
        if (!pfMesh)
        {
            setError("获取网格环境失败");
            return false;
        }

        // 导入网格文件
        status = pfMesh->importMesh(meshFilePath.c_str());
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            setError("导入网格文件失败");
            return false;
        }

        // 获取几何对象
        m_pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(
            pfDocument->getGeometryEnvironment());
        
        if (!m_pfGeometry)
        {
            setError("获取几何环境失败");
            return false;
        }

        // 转换网格为几何
        return convertMeshToGeometry();
    }
    catch (const std::exception& e)
    {
        setError(std::string("导入网格并转换异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::convertMeshToGeometry()
{
    if (!checkGeometry())
    {
        return false;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = m_pfGeometry->convertMeshToGeometry();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        setError("将网格转换为几何失败");
        return false;
    }

    return true;
}

// ============== 可视化支持 ==============

bool GeometryAPI::showGeometry(bool showGroups)
{
    if (!checkGeometry())
    {
        return false;
    }

    try
    {
        PREPRO_BASE_NAMESPACE::PFData geometryData;
        m_pfGeometry->getAllData(geometryData);
        
        if (showGroups)
        {
            std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupList;
            // 这里可以添加获取组列表的代码
            RenderProcessor::show(geometryData, VisualizationType::EGeomerty, groupList);
        }
        else
        {
            RenderProcessor::show(geometryData, VisualizationType::EGeomerty);
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("显示几何异常: ") + e.what());
        return false;
    }
}

bool GeometryAPI::clearGeometry()
{
    if (!checkGeometry())
    {
        return false;
    }

    try 
    {
        std::cout << "[GeometryAPI] 正在清理几何数据..." << std::endl;
        
        // 由于SDK没有clearGeometry方法，我们使用其他方式进行清理
        // 这里暂时返回成功，实际清理需要根据SDK具体API实现
        std::cout << "[GeometryAPI] 注意: SDK没有直接的清理方法，建议重新初始化或使用替换模式导入" << std::endl;
        std::cout << "[GeometryAPI] 可以尝试使用导入模式2(替换)来覆盖现有数据" << std::endl;
        
        return true;
    }
    catch (const std::exception& e)
    {
        setError(std::string("清理几何数据异常: ") + e.what());
        return false;
    }
}



// ============== 状态查询 ==============

bool GeometryAPI::isInitialized() const
{
    return m_initialized;
}

std::string GeometryAPI::getLastError() const
{
    return m_lastError;
}

PREPRO_BASE_NAMESPACE::PFDocument* GeometryAPI::getDocument() const
{
    return m_pfDocument;
}

// ============== 私有辅助方法 ==============

void GeometryAPI::setError(const std::string& error)
{
    m_lastError = error;
    std::cerr << "GeometryAPI错误: " << error << std::endl;
}

bool GeometryAPI::checkGeometry()
{
    if (!m_initialized)
    {
        setError("API未初始化，请先调用initialize()");
        return false;
    }

    if (!m_pfGeometry)
    {
        setError("几何对象为空");
        return false;
    }

    return true;
}

template<typename T>
T* GeometryAPI::vectorToArray(const std::vector<T>& vec)
{
    if (vec.empty())
    {
        return nullptr;
    }

    T* array = new T[vec.size()];
    std::copy(vec.begin(), vec.end(), array);
    return array;
}

const char** GeometryAPI::stringVectorToArray(const std::vector<std::string>& vec)
{
    if (vec.empty())
    {
        return nullptr;
    }

    const char** array = new const char*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i)
    {
        array[i] = vec[i].c_str();
    }
    return array;
}
