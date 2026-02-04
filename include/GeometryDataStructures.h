#ifndef GEOMETRY_DATA_STRUCTURES_H
#define GEOMETRY_DATA_STRUCTURES_H

#include <vector>
#include <string>
#include <memory>

/**
 * @brief 3D点坐标结构
 */
struct Point3D
{
    double x, y, z;
    
    Point3D() : x(0.0), y(0.0), z(0.0) {}
    Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    Point3D operator+(const Point3D& other) const {
        return Point3D(x + other.x, y + other.y, z + other.z);
    }
    
    Point3D operator-(const Point3D& other) const {
        return Point3D(x - other.x, y - other.y, z - other.z);
    }
    
    Point3D operator*(double scale) const {
        return Point3D(x * scale, y * scale, z * scale);
    }
    
    Point3D operator/(double scale) const {
        return Point3D(x / scale, y / scale, z / scale);
    }
};

/**
 * @brief 3D包围盒结构
 */
struct BoundingBox3D
{
    Point3D center;     // 包围盒中心
    Point3D size;       // 包围盒尺寸 (width, height, depth)
    Point3D minPoint;   // 最小点坐标
    Point3D maxPoint;   // 最大点坐标
    
    BoundingBox3D() = default;
    BoundingBox3D(const Point3D& center_, const Point3D& size_, const Point3D& min_, const Point3D& max_)
        : center(center_), size(size_), minPoint(min_), maxPoint(max_) {}
    
    /**
     * @brief 获取包围盒体积
     */
    double getVolume() const {
        return size.x * size.y * size.z;
    }
    
    /**
     * @brief 获取包围盒表面积
     */
    double getSurfaceArea() const {
        return 2.0 * (size.x * size.y + size.y * size.z + size.z * size.x);
    }
};

/**
 * @brief 几何面类
 */
class GeometryFace
{
public:
    // 构造函数
    GeometryFace() = default;
    GeometryFace(unsigned int id, const std::string& name = "");
    
    // 基本信息
    void setId(unsigned int id) { m_id = id; }
    unsigned int getId() const { return m_id; }
    
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    
    void setGroupIndex(int index) { m_groupIndex = index; }
    int getGroupIndex() const { return m_groupIndex; }
    
    void setElementCount(size_t count) { m_elementCount = count; }
    size_t getElementCount() const { return m_elementCount; }
    
    // 几何属性
    void setVertices(const std::vector<Point3D>& vertices) { m_vertices = vertices; }
    const std::vector<Point3D>& getVertices() const { return m_vertices; }
    size_t getVertexCount() const { return m_vertices.size(); }
    
    void setCentroid(const Point3D& centroid) { m_centroid = centroid; }
    const Point3D& getCentroid() const { return m_centroid; }
    
    void setBoundingBox(const BoundingBox3D& bbox) { m_boundingBox = bbox; }
    const BoundingBox3D& getBoundingBox() const { return m_boundingBox; }
    
    void setArea(double area) { m_area = area; }
    double getArea() const { return m_area; }
    
    // 计算方法
    void calculateCentroid();
    void calculateBoundingBox();
    void calculateArea();
    
    // 输出信息
    void printInfo() const;
    
private:
    unsigned int m_id = 0;
    std::string m_name;
    int m_groupIndex = -1;
    size_t m_elementCount = 0;
    
    std::vector<Point3D> m_vertices;
    Point3D m_centroid;
    BoundingBox3D m_boundingBox;
    double m_area = 0.0;
};

/**
 * @brief 几何体类
 */
class GeometryVolume
{
public:
    // 构造函数
    GeometryVolume() = default;
    GeometryVolume(unsigned int id, const std::string& name = "");
    
    // 基本信息
    void setId(unsigned int id) { m_id = id; }
    unsigned int getId() const { return m_id; }
    
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    
    void setIndex(int index) { m_index = index; }
    int getIndex() const { return m_index; }
    
    // 面组管理
    void addFace(std::shared_ptr<GeometryFace> face);
    void addFace(const GeometryFace& face);
    const std::vector<std::shared_ptr<GeometryFace>>& getFaces() const { return m_faces; }
    size_t getFaceCount() const { return m_faces.size(); }
    std::shared_ptr<GeometryFace> getFace(size_t index) const;
    
    // 几何属性
    void setCentroid(const Point3D& centroid) { m_centroid = centroid; }
    const Point3D& getCentroid() const { return m_centroid; }
    
    void setMassCentroid(const Point3D& massCentroid) { m_massCentroid = massCentroid; }
    const Point3D& getMassCentroid() const { return m_massCentroid; }
    
    void setBoundingBox(const BoundingBox3D& bbox) { m_boundingBox = bbox; }
    const BoundingBox3D& getBoundingBox() const { return m_boundingBox; }
    
    void setVolume(double volume) { m_volume = volume; }
    double getVolume() const { return m_volume; }
    
    void setCalculatedVolume(double calcVolume) { m_calculatedVolume = calcVolume; }
    double getCalculatedVolume() const { return m_calculatedVolume; }
    
    // 统计信息
    void setTotalVertexCount(size_t count) { m_totalVertexCount = count; }
    size_t getTotalVertexCount() const { return m_totalVertexCount; }
    
    void setTriangleCount(size_t count) { m_triangleCount = count; }
    size_t getTriangleCount() const { return m_triangleCount; }
    
    // 计算方法
    void calculateProperties();
    void calculateBoundingBoxFromFaces();
    double calculateTotalSurfaceArea() const;
    
    // 输出信息
    void printInfo() const;
    void printDetailedInfo() const;
    
private:
    unsigned int m_id = 0;
    std::string m_name;
    int m_index = -1;
    
    std::vector<std::shared_ptr<GeometryFace>> m_faces;
    
    Point3D m_centroid;
    Point3D m_massCentroid;
    BoundingBox3D m_boundingBox;
    double m_volume = 0.0;
    double m_calculatedVolume = 0.0;
    
    size_t m_totalVertexCount = 0;
    size_t m_triangleCount = 0;
};

/**
 * @brief 几何模型类 - 管理所有体对象
 */
class GeometryModel
{
public:
    GeometryModel() = default;
    
    // 体对象管理
    void addVolume(std::shared_ptr<GeometryVolume> volume);
    void addVolume(const GeometryVolume& volume);
    const std::vector<std::shared_ptr<GeometryVolume>>& getVolumes() const { return m_volumes; }
    size_t getVolumeCount() const { return m_volumes.size(); }
    std::shared_ptr<GeometryVolume> getVolume(size_t index) const;
    
    // 统计信息
    size_t getTotalFaceCount() const;
    double getTotalVolume() const;
    double getTotalSurfaceArea() const;
    
    // 清理
    void clear();
    
    // 输出信息
    void printSummary() const;
    void printDetailedInfo() const;
    
private:
    std::vector<std::shared_ptr<GeometryVolume>> m_volumes;
};

// ============= 新的数据结构：用于存储几何属性数据 =============

/**
 * @brief 面的几何属性数据结构
 * 用于存储面的所有几何属性信息
 */
struct FaceData
{
    // 基本信息
    unsigned int id = 0;                    ///< 面ID
    std::string name;                       ///< 面名称
    int groupIndex = -1;                    ///< 所属组索引
    size_t elementCount = 0;                ///< 元素数量
    
    // 几何属性
    std::vector<Point3D> vertices;          ///< 顶点坐标列表
    Point3D centroid;                       ///< 面心坐标
    BoundingBox3D boundingBox;              ///< 包围盒
    double area = 0.0;                      ///< 面积
    
    // 构造函数
    FaceData() = default;
    FaceData(unsigned int faceId, const std::string& faceName = "")
        : id(faceId), name(faceName) {}
    
    // 清空数据
    void clear()
    {
        id = 0;
        name.clear();
        groupIndex = -1;
        elementCount = 0;
        vertices.clear();
        centroid = Point3D();
        boundingBox = BoundingBox3D();
        area = 0.0;
    }
};

/**
 * @brief 体的几何属性数据结构
 * 用于存储体的所有几何属性信息，包括面组
 */
struct VolumeData
{
    // 基本信息
    unsigned int id = 0;                    ///< 体ID
    std::string name;                        ///< 体名称
    int index = -1;                          ///< 索引
    
    // 面组（存储面的几何属性数据）
    std::vector<FaceData> faceGroup;        ///< 面组列表
    
    // 几何属性
    Point3D centroid;                        ///< 几何重心
    Point3D massCentroid;                   ///< 质量重心
    BoundingBox3D boundingBox;              ///< 包围盒
    double volume = 0.0;                    ///< 体积
    double calculatedVolume = 0.0;          ///< 计算得到的体积
    
    // 统计信息
    size_t totalVertexCount = 0;             ///< 总顶点数
    size_t triangleCount = 0;                ///< 三角形数量
    
    // 构造函数
    VolumeData() = default;
    VolumeData(unsigned int volumeId, const std::string& volumeName = "")
        : id(volumeId), name(volumeName) {}
    
    // 面组管理方法
    void addFace(const FaceData& face)
    {
        faceGroup.push_back(face);
    }
    
    void addFace(FaceData&& face)
    {
        faceGroup.push_back(std::move(face));
    }
    
    size_t getFaceCount() const
    {
        return faceGroup.size();
    }
    
    const FaceData* getFace(size_t index) const
    {
        if (index < faceGroup.size()) {
            return &faceGroup[index];
        }
        return nullptr;
    }
    
    FaceData* getFace(size_t index)
    {
        if (index < faceGroup.size()) {
            return &faceGroup[index];
        }
        return nullptr;
    }
    
    // 清空数据
    void clear()
    {
        id = 0;
        name.clear();
        index = -1;
        faceGroup.clear();
        centroid = Point3D();
        massCentroid = Point3D();
        boundingBox = BoundingBox3D();
        volume = 0.0;
        calculatedVolume = 0.0;
        totalVertexCount = 0;
        triangleCount = 0;
    }
};

#endif // GEOMETRY_DATA_STRUCTURES_H
