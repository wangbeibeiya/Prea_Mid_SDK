// Revision: $Id: meshDemo.h,v 1.0 2025/02/21 13:15:20 liujing Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2021] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technologies, Inc.
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
#include <base/common/pfConstDefinitions.h>
#include <base/common/pfEnumeration.h>
#include <mesh/common/pfConstDefinitions.h>

START_PREPRO_BASE_NAMESPACE
class PFDocument;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_MESH_NAMESPACE
class PFMesh;
class PFBoxDensitySetting;
class PFSphereDensitySetting;
class PFCylinderDensitySetting;
class PFMeshQuality;
END_PREPRO_MESH_NAMESPACE


class MeshDemo
{
public:
    static void meshDemo(const std::string& path);
    
    // 网格参数设置函数 - 供外部调用（如树节点管理器）
    static void setMeshGlobalParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument);
    static void setMeshLocalSizeParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument);

private:

    static void setShrinkwrapParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument);

    static PREPRO_BASE_NAMESPACE::PFStatus createSurfaceMesh(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh);

    static PREPRO_BASE_NAMESPACE::PFStatus createVolumeMeshBySurfaceMesh(const std::string& path);

    static PREPRO_BASE_NAMESPACE::PFStatus createVolumeMeshByGeometryNoInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh);

    static PREPRO_BASE_NAMESPACE::PFStatus createVolumeMeshBySurfaceMeshNoInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh);

    static PREPRO_BASE_NAMESPACE::PFStatus createVolumeMeshBySurfaceMeshNeedInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh);

    static void importMesh(const std::string& path);

    static void exportMesh(const std::string& path);

    static void createShrinkWrapMesh(const std::string& path);

    static void Remesh(const std::string& path);

    static void computeLeakDetection(const std::string& path);
    static void DensityDemo(const std::string& path);

    static void thinCut(const std::string& path);

    static void faceMeshParametersDemo(const std::string& path);
    static void edgeMeshParameter(const std::string& path);
    static void inflationSettingDemo(const std::string& path);

    static void periodicFace(const std::string& path);

    static void convertToPolyhedralMesh(const std::string& path);
    static void smoothVolumeMesh(const std::string& path);
    static void meshQualityDemo(const std::string& path);
    static void getOrthogonality(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getQuality(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getVolume(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getSkewness(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getAspectRatio(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getOrthogonalityFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getAspectRatioFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getOrthogonalitySkewFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);
    static void getJacobian(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality);

    static void OpenDensity(const std::string& path);
    static void DensityTest(const std::string& path);
    static void CylinderDensityTest(const std::string& path);
    static bool DensityTestEditBox(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFBoxDensitySetting* boxDensity);
    static bool DensityTestEditSphere(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFSphereDensitySetting* sphereDensity);
    static bool DensityTestEditCylinder(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting* cylinderDensity);
    
    static bool approximatelyEqual(double a, double b);

    static void convertGeometryToMesh(const std::string& examplePath);

    static void cutPlaneDemo(const std::string& path);
};