#include "../include/GeometryDataStructures.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

// ============= GeometryFace Implementation =============

GeometryFace::GeometryFace(unsigned int id, const std::string& name)
    : m_id(id), m_name(name)
{
}

void GeometryFace::calculateCentroid()
{
    if (m_vertices.empty()) return;
    
    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
    for (const auto& vertex : m_vertices)
    {
        sum_x += vertex.x;
        sum_y += vertex.y;
        sum_z += vertex.z;
    }
    
    m_centroid.x = sum_x / m_vertices.size();
    m_centroid.y = sum_y / m_vertices.size();
    m_centroid.z = sum_z / m_vertices.size();
}

void GeometryFace::calculateBoundingBox()
{
    if (m_vertices.empty()) return;
    
    double min_x = m_vertices[0].x, max_x = m_vertices[0].x;
    double min_y = m_vertices[0].y, max_y = m_vertices[0].y;
    double min_z = m_vertices[0].z, max_z = m_vertices[0].z;
    
    for (const auto& vertex : m_vertices)
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
    
    m_boundingBox = BoundingBox3D(center, size, minPt, maxPt);
}

void GeometryFace::calculateArea()
{
    if (m_vertices.size() < 3)
    {
        m_area = 0.0;
        return;
    }
    
    double totalArea = 0.0;
    const Point3D& basePoint = m_vertices[0];
    
    // 使用三角剖分计算面积
    for (size_t i = 1; i < m_vertices.size() - 1; i++)
    {
        const Point3D& p1 = m_vertices[i];
        const Point3D& p2 = m_vertices[i + 1];
        
        // 计算向量 AB 和 AC
        Point3D ab = p1 - basePoint;
        Point3D ac = p2 - basePoint;
        
        // 计算叉积
        Point3D cross(
            ab.y * ac.z - ab.z * ac.y,
            ab.z * ac.x - ab.x * ac.z,
            ab.x * ac.y - ab.y * ac.x
        );
        
        // 叉积的模长就是平行四边形面积，三角形面积是一半
        double triangleArea = 0.5 * std::sqrt(cross.x * cross.x + cross.y * cross.y + cross.z * cross.z);
        totalArea += triangleArea;
    }
    
    m_area = totalArea;
}

void GeometryFace::printInfo() const
{
    std::cout << "        真实面 ID: " << m_id;
    if (!m_name.empty()) {
        std::cout << " (名称: " << m_name << ")";
    }
    std::cout << std::endl;
    
    std::cout << "          元素数量: " << m_elementCount << std::endl;
    std::cout << "          顶点数量: " << m_vertices.size() << std::endl;
    std::cout << "          面心坐标: (" << m_centroid.x << ", " << m_centroid.y << ", " << m_centroid.z << ")" << std::endl;
    std::cout << "          面积: " << m_area << std::endl;
    std::cout << "          包围盒中心: (" << m_boundingBox.center.x << ", " << m_boundingBox.center.y << ", " << m_boundingBox.center.z << ")" << std::endl;
    std::cout << "          包围盒尺寸: (" << m_boundingBox.size.x << ", " << m_boundingBox.size.y << ", " << m_boundingBox.size.z << ")" << std::endl;
    std::cout << "          包围盒范围: Min(" << m_boundingBox.minPoint.x << ", " << m_boundingBox.minPoint.y << ", " << m_boundingBox.minPoint.z 
              << ") ~ Max(" << m_boundingBox.maxPoint.x << ", " << m_boundingBox.maxPoint.y << ", " << m_boundingBox.maxPoint.z << ")" << std::endl;
}

// ============= GeometryVolume Implementation =============

GeometryVolume::GeometryVolume(unsigned int id, const std::string& name)
    : m_id(id), m_name(name)
{
}

void GeometryVolume::addFace(std::shared_ptr<GeometryFace> face)
{
    if (face) {
        m_faces.push_back(face);
    }
}

void GeometryVolume::addFace(const GeometryFace& face)
{
    m_faces.push_back(std::make_shared<GeometryFace>(face));
}

std::shared_ptr<GeometryFace> GeometryVolume::getFace(size_t index) const
{
    if (index < m_faces.size()) {
        return m_faces[index];
    }
    return nullptr;
}

void GeometryVolume::calculateProperties()
{
    calculateBoundingBoxFromFaces();
}

void GeometryVolume::calculateBoundingBoxFromFaces()
{
    if (m_faces.empty()) return;
    
    // 收集所有面的顶点来计算整体包围盒
    std::vector<Point3D> allVertices;
    for (const auto& face : m_faces)
    {
        if (face) {
            const auto& faceVertices = face->getVertices();
            allVertices.insert(allVertices.end(), faceVertices.begin(), faceVertices.end());
        }
    }
    
    if (allVertices.empty()) return;
    
    double min_x = allVertices[0].x, max_x = allVertices[0].x;
    double min_y = allVertices[0].y, max_y = allVertices[0].y;
    double min_z = allVertices[0].z, max_z = allVertices[0].z;
    
    for (const auto& vertex : allVertices)
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
    
    m_boundingBox = BoundingBox3D(center, size, minPt, maxPt);
}

double GeometryVolume::calculateTotalSurfaceArea() const
{
    double totalArea = 0.0;
    for (const auto& face : m_faces)
    {
        if (face) {
            totalArea += face->getArea();
        }
    }
    return totalArea;
}

void GeometryVolume::printInfo() const
{
    std::cout << "\n体对象 #" << (m_index + 1) << ":" << std::endl;
    std::cout << "  名称: " << (m_name.empty() ? "未命名" : m_name) << std::endl;
    std::cout << "  ID: " << m_id << std::endl;
    std::cout << "  包含面数: " << m_faces.size() << std::endl;
    std::cout << "  总顶点数: " << m_totalVertexCount << std::endl;
    std::cout << "  几何重心: (" << m_centroid.x << ", " << m_centroid.y << ", " << m_centroid.z << ")" << std::endl;
    std::cout << "  质量重心: (" << m_massCentroid.x << ", " << m_massCentroid.y << ", " << m_massCentroid.z << ")" << std::endl;
    std::cout << "  包围盒中心: (" << m_boundingBox.center.x << ", " << m_boundingBox.center.y << ", " << m_boundingBox.center.z << ")" << std::endl;
    std::cout << "  包围盒尺寸: (" << m_boundingBox.size.x << ", " << m_boundingBox.size.y << ", " << m_boundingBox.size.z << ")" << std::endl;
    std::cout << "  估算体积: " << m_volume << std::endl;
    std::cout << "  计算体积: " << m_calculatedVolume << std::endl;
    std::cout << "  表面积: " << calculateTotalSurfaceArea() << std::endl;
}

void GeometryVolume::printDetailedInfo() const
{
    printInfo();
    
    std::cout << "  详细面信息:" << std::endl;
    for (size_t i = 0; i < m_faces.size(); ++i)
    {
        if (m_faces[i]) {
            std::cout << "    面 " << (i + 1) << ":" << std::endl;
            m_faces[i]->printInfo();
        }
    }
}

// ============= GeometryModel Implementation =============

void GeometryModel::addVolume(std::shared_ptr<GeometryVolume> volume)
{
    if (volume) {
        m_volumes.push_back(volume);
    }
}

void GeometryModel::addVolume(const GeometryVolume& volume)
{
    m_volumes.push_back(std::make_shared<GeometryVolume>(volume));
}

std::shared_ptr<GeometryVolume> GeometryModel::getVolume(size_t index) const
{
    if (index < m_volumes.size()) {
        return m_volumes[index];
    }
    return nullptr;
}

size_t GeometryModel::getTotalFaceCount() const
{
    size_t totalFaces = 0;
    for (const auto& volume : m_volumes)
    {
        if (volume) {
            totalFaces += volume->getFaceCount();
        }
    }
    return totalFaces;
}

double GeometryModel::getTotalVolume() const
{
    double totalVolume = 0.0;
    for (const auto& volume : m_volumes)
    {
        if (volume) {
            totalVolume += volume->getVolume();
        }
    }
    return totalVolume;
}

double GeometryModel::getTotalSurfaceArea() const
{
    double totalArea = 0.0;
    for (const auto& volume : m_volumes)
    {
        if (volume) {
            totalArea += volume->calculateTotalSurfaceArea();
        }
    }
    return totalArea;
}

void GeometryModel::clear()
{
    m_volumes.clear();
}

void GeometryModel::printSummary() const
{
    std::cout << "\n======== 几何模型统计信息 ========" << std::endl;
    std::cout << "体对象数量: " << m_volumes.size() << std::endl;
    std::cout << "总面数量: " << getTotalFaceCount() << std::endl;
    std::cout << "总体积: " << getTotalVolume() << std::endl;
    std::cout << "总表面积: " << getTotalSurfaceArea() << std::endl;
    std::cout << "================================" << std::endl;
}

void GeometryModel::printDetailedInfo() const
{
    printSummary();
    
    std::cout << "\n详细信息:" << std::endl;
    for (const auto& volume : m_volumes)
    {
        if (volume) {
            volume->printDetailedInfo();
        }
    }
}
