// Revision: $Id: GeometryAPI.h,v 1.0 2025/02/27 10:00:00 assistant Exp $
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
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <geometry/common/pfGeometryMacro.h>
#include <base/common/pfConstDefinitions.h>

START_PREPRO_BASE_NAMESPACE
class PFApplication;
class PFData;
class PFDocument;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_GEOMETRY_NAMESPACE
class PFGeometry;
END_PREPRO_GEOMETRY_NAMESPACE

/**
 * @brief 几何操作API封装类
 * 
 * 本类将Pera CFD SDK中的几何操作功能封装成统一的API接口，
 * 提供简单易用的几何建模、修复、变换等功能。
 */
class GeometryAPI
{
public:
    /**
     * @brief 构造函数
     * @param examplePath 示例文件路径
     */
    explicit GeometryAPI(const std::string& examplePath = "");
    
    /**
     * @brief 析构函数
     */
    ~GeometryAPI();

    // ============== 初始化和文件操作 ==============
    
    /**
     * @brief 初始化几何环境
     * @return 是否成功初始化
     */
    bool initialize();
    
    /**
     * @brief 导入几何文件
     * @param filePath 文件路径
     * @param importMode 导入模式：1-首次导入，2-替换模式，3-追加模式
     * @return 是否成功导入
     */
    bool importGeometry(const std::string& filePath, int importMode = 1);
    
    /**
     * @brief 导入网格文件
     * @param filePath 网格文件路径
     * @return 是否成功导入
     */
    bool importMesh(const std::string& filePath);
    
    /**
     * @brief 导入后处理结果
     * @param filePath 结果文件路径
     * @return 是否成功导入
     */
    bool importPostProcessResult(const std::string& filePath);
    
    /**
     * @brief 导出几何文件
     * @param filePath 文件路径
     * @return 是否成功导出
     */
    bool exportGeometry(const std::string& filePath);
    
    /**
     * @brief 保存项目文档
     * @param filePath 保存路径（可选，如果为空则保存到当前文档路径）
     * @return 是否成功保存
     */
    bool saveDocument(const std::string& filePath = "");
    
    /**
     * @brief 获取所有几何数据
     * @param data 输出的几何数据
     * @return 是否成功获取
     */
    bool getAllData(PREPRO_BASE_NAMESPACE::PFData& data);

    // ============== 几何修复功能 ==============
    
    /**
     * @brief 快速修复几何
     * @param stitchTolerance 缝合容差，默认1e-5
     * @return 是否修复成功
     */
    bool quickRepair(double stitchTolerance = 1e-5);
    
    /**
     * @brief 查找体积
     * @return 找到的体积数量，-1表示失败
     */
    int findVolumes();
    
    /**
     * @brief 缝合两个面
     * @param faceId1 第一个面的ID
     * @param faceId2 第二个面的ID
     * @param tolerance 容差，默认0.001
     * @return 是否缝合成功
     */
    bool stitchTwoFaces(unsigned int faceId1, unsigned int faceId2, double tolerance = 0.001);
    
    /**
     * @brief 去除特征
     * @param tolerance 容差，默认1e-6
     * @return 是否成功去除特征
     */
    bool defeature(double tolerance = 1e-6);

    // ============== 孔洞处理 ==============
    
    /**
     * @brief 缝合孔洞
     * @param holeId 孔洞ID
     * @return 是否缝合成功
     */
    bool stitchHole(unsigned int holeId);
    
    /**
     * @brief 填充孔洞
     * @param holeId 孔洞ID
     * @return 是否填充成功
     */
    bool fillHole(unsigned int holeId);
    
    /**
     * @brief 填充所有孔洞
     * @return 是否填充成功
     */
    bool fillAllHoles();
    
    /**
     * @brief 填充指定面的孔洞
     * @param faceIds 面ID列表
     * @return 是否填充成功
     */
    bool fillFacesHoles(const std::vector<unsigned int>& faceIds);

    // ============== 面操作 ==============
    
    /**
     * @brief 两个面相交
     * @param faceId1 第一个面的ID
     * @param faceId2 第二个面的ID
     * @param tolerance 容差，默认0.001
     * @return 是否相交成功
     */
    bool intersectTwoFaces(unsigned int faceId1, unsigned int faceId2, double tolerance = 0.001);
    
    /**
     * @brief 合并面
     * @param faceIds 要合并的面ID列表
     * @return 是否合并成功
     */
    bool mergeFaces(const std::vector<unsigned int>& faceIds);
    
    /**
     * @brief 通过边创建面
     * @param edgeIds 边ID列表
     * @return 是否创建成功
     */
    bool createFaceByEdges(const std::vector<unsigned int>& edgeIds);
    
    /**
     * @brief 通过点创建面
     * @param coordinates 点坐标数组 (x1,y1,z1,x2,y2,z2,...)
     * @param pointCount 点的数量
     * @return 是否创建成功
     */
    bool createFaceByPoints(const std::vector<double>& coordinates, unsigned int pointCount);
    
    /**
     * @brief 用边分割面
     * @param faceId 面ID
     * @param edgeId 边ID
     * @return 是否分割成功
     */
    bool splitFaceByEdge(unsigned int faceId, unsigned int edgeId);

    // ============== 边操作 ==============
    
    /**
     * @brief 创建边
     * @param startPointId 起点ID
     * @param endPointId 终点ID
     * @return 是否创建成功
     */
    bool createEdge(unsigned int startPointId, unsigned int endPointId);
    
    /**
     * @brief 分割边
     * @param edgeId 边ID
     * @param parameter 分割参数 (0.0-1.0)
     * @return 是否分割成功
     */
    bool splitEdge(unsigned int edgeId, double parameter);
    
    /**
     * @brief 合并边
     * @param edgeIds 要合并的边ID列表
     * @return 是否合并成功
     */
    bool mergeEdges(const std::vector<unsigned int>& edgeIds);

    // ============== 几何变换 ==============
    
    /**
     * @brief 缩放几何实体
     * @param entityIds 实体ID列表
     * @param origin 缩放原点 [x,y,z]
     * @param scaleFactor 缩放因子
     * @param onlyTransform 是否仅变换（不复制），默认true
     * @return 是否缩放成功
     */
    bool scale(const std::vector<unsigned int>& entityIds, 
               const std::vector<double>& origin, 
               double scaleFactor, 
               bool onlyTransform = true);
    
    /**
     * @brief 镜像几何实体
     * @param entityIds 实体ID列表
     * @param origin 镜像原点 [x,y,z]
     * @param direction 镜像方向 [x,y,z]
     * @param copyAndTransform 是否复制并变换，默认false
     * @return 是否镜像成功
     */
    bool mirror(const std::vector<unsigned int>& entityIds,
                const std::vector<double>& origin,
                const std::vector<double>& direction,
                bool copyAndTransform = false);
    
    /**
     * @brief 旋转几何实体
     * @param entityIds 实体ID列表
     * @param origin 旋转中心 [x,y,z]
     * @param axis 旋转轴 [x,y,z]
     * @param angle 旋转角度（度）
     * @param onlyTransform 是否仅变换（不复制），默认true
     * @return 是否旋转成功
     */
    bool rotate(const std::vector<unsigned int>& entityIds,
                const std::vector<double>& origin,
                const std::vector<double>& axis,
                double angle,
                bool onlyTransform = true);
    
    /**
     * @brief 平移几何实体
     * @param entityIds 实体ID列表
     * @param offset 偏移量 [x,y,z]
     * @param copyAndTransform 是否复制并变换，默认false
     * @return 是否平移成功
     */
    bool translate(const std::vector<unsigned int>& entityIds,
                   const std::vector<double>& offset,
                   bool copyAndTransform = false);

    // ============== 基本几何体创建 ==============
    
    /**
     * @brief 创建立方体
     * @param name 立方体名称
     * @param startPoint 起始点 [x,y,z]
     * @param endPoint 结束点 [x,y,z]
     * @return 是否创建成功
     */
    bool createCube(const std::string& name,
                    const std::vector<float>& startPoint,
                    const std::vector<float>& endPoint);
    
    /**
     * @brief 创建圆柱体
     * @param name 圆柱体名称
     * @param startPoint 起始点 [x,y,z]
     * @param endPoint 结束点 [x,y,z]
     * @param radius 半径
     * @return 是否创建成功
     */
    bool createCylinder(const std::string& name,
                        const std::vector<float>& startPoint,
                        const std::vector<float>& endPoint,
                        float radius);
    
    /**
     * @brief 创建球体
     * @param name 球体名称
     * @param center 中心点 [x,y,z]
     * @param radius 半径
     * @return 是否创建成功
     */
    bool createSphere(const std::string& name,
                      const std::vector<float>& center,
                      float radius);
    
    /**
     * @brief 创建半球
     * @param name 半球名称
     * @param center 中心点 [x,y,z]
     * @param direction 方向 [x,y,z]
     * @param radius 半径
     * @return 是否创建成功
     */
    bool createHemisphere(const std::string& name,
                          const std::vector<float>& center,
                          const std::vector<float>& direction,
                          float radius);

    // ============== 高级操作 ==============
    
    /**
     * @brief 拉伸操作
     * @param entityIds 实体ID列表
     * @param distance 拉伸距离
     * @param useNormal 是否使用法向，默认true
     * @return 是否拉伸成功
     */
    bool extrude(const std::vector<unsigned int>& entityIds, 
                 double distance, 
                 bool useNormal = true);
    
    /**
     * @brief 压印操作
     * @param faceIds 面ID列表
     * @return 是否压印成功
     */
    bool imprint(const std::vector<unsigned int>& faceIds);
    
    /**
     * @brief 投影操作
     * @param sourceId 源实体ID
     * @param targetId 目标实体ID
     * @return 是否投影成功
     */
    bool project(unsigned int sourceId, unsigned int targetId);

    // ============== 实体管理 ==============
    
    /**
     * @brief 删除实体
     * @param entityIds 实体ID列表
     * @param keepAffiliated 是否保留相关实体，默认true
     * @return 是否删除成功
     */
    bool deleteEntities(const std::vector<unsigned int>& entityIds, 
                        bool keepAffiliated = true);
    
    /**
     * @brief 添加实体到组
     * @param entityIds 实体ID列表
     * @param groupName 组名
     * @return 是否添加成功
     */
    bool addEntitiesToGroup(const std::vector<unsigned int>& entityIds, 
                            const std::string& groupName);

    // ============== 体积操作 ==============
    
    /**
     * @brief 创建体积
     * @param groupNames 组名列表
     * @return 是否创建成功
     */
    bool createVolume(const std::vector<std::string>& groupNames);
    
    /**
     * @brief 分离体积
     * @param volumeNames 体积名称列表
     * @return 是否分离成功
     */
    bool detachVolumes(const std::vector<std::string>& volumeNames);
    
    /**
     * @brief 删除体积
     * @param volumeNames 体积名称列表
     * @return 是否删除成功
     */
    bool deleteVolumes(const std::vector<std::string>& volumeNames);

    // ============== 命名操作 ==============
    
    /**
     * @brief 重命名面组
     * @param oldName 旧名称
     * @param newName 新名称
     * @return 是否重命名成功
     */
    bool renameFaceGroup(const std::string& oldName, const std::string& newName);
    
    /**
     * @brief 重命名体积
     * @param oldName 旧名称
     * @param newName 新名称
     * @return 是否重命名成功
     */
    bool renameVolume(const std::string& oldName, const std::string& newName);
    
    /**
     * @brief 删除面组
     * @param groupNames 组名列表
     * @return 是否删除成功
     */
    bool deleteFaceGroups(const std::vector<std::string>& groupNames);

    // ============== 网格转换 ==============
    
    /**
     * @brief 导入网格文件并转换为几何
     * @param meshFilePath 网格文件路径
     * @return 是否转换成功
     */
    bool importMeshAndConvertToGeometry(const std::string& meshFilePath);
    
    /**
     * @brief 将网格转换为几何
     * @return 是否转换成功
     */
    bool convertMeshToGeometry();

    // ============== 可视化支持 ==============
    
    /**
     * @brief 显示几何
     * @param showGroups 是否显示组，默认false
     * @return 是否显示成功
     */
    bool showGeometry(bool showGroups = false);
    
    /**
     * @brief 清理几何数据，为下一次导入做准备
     * @return 是否成功
     */
    bool clearGeometry();

    // ============== 状态查询 ==============
    
    /**
     * @brief 检查API是否已初始化
     * @return 是否已初始化
     */
    bool isInitialized() const;
    
    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 获取PFDocument指针（用于网格处理等）
     * @return PFDocument指针，如果未初始化则返回nullptr
     */
    PREPRO_BASE_NAMESPACE::PFDocument* getDocument() const;

private:
    // 私有成员变量
    std::string m_examplePath;                                    ///< 示例文件路径
    PREPRO_BASE_NAMESPACE::PFApplication* m_pfApplication;       ///< PF应用程序对象
    PREPRO_BASE_NAMESPACE::PFDocument* m_pfDocument;             ///< PF文档对象
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* m_pfGeometry;         ///< PF几何对象
    bool m_initialized;                                           ///< 初始化状态
    std::string m_lastError;                                      ///< 最后的错误信息
    
    // 私有辅助方法
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setError(const std::string& error);
    
    /**
     * @brief 检查几何对象是否有效
     * @return 是否有效
     */
    bool checkGeometry();
    
    /**
     * @brief 将vector转换为数组指针
     * @param vec 输入向量
     * @return 数组指针（需要手动释放）
     */
    template<typename T>
    T* vectorToArray(const std::vector<T>& vec);
    
    /**
     * @brief 将string vector转换为const char**
     * @param vec 输入字符串向量
     * @return const char**指针（需要手动释放）
     */
    const char** stringVectorToArray(const std::vector<std::string>& vec);
};
