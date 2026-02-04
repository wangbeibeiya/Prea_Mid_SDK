// Revision: $Id: faceMeshParametersDemo.cpp,v 1.0 2025/04/01 20:15:20 Leon Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2025] PeraGlobal Technologies, Inc.
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

#include "meshDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <base/pfGroupData.h>
#include <mesh/pfmesh.h>
#include <mesh/pfGlobalParameters.h>
#include <mesh/pfMeshBuilder.h>
#include <mesh/pfFaceMeshParameters.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"

void MeshDemo::faceMeshParametersDemo(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (nullptr != geomertyBuilder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(geomertyBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }
    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr != meshBuilder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    std::string filePath = examplePath + "mesh\\test_face_mesh_param.ppcf";
    PFDocument* pfDocument = pfApplication.openDocument(filePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return;
    }

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        std::cout << "!!! Geometry is null." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        return;
    }

    PREPRO_MESH_NAMESPACE::PFFaceMeshParameters* pfFaceMeshParameters = pfMesh->getFaceMeshParameters();
    int faceIds[2] = { 22, 24 };
    pfFaceMeshParameters->setFaceMeshSetting(2, faceIds, 0.988, PFFaceMeshType::ETriangles);
    double maxSizeValue = 0.0;
    PREPRO_BASE_NAMESPACE::PFFaceMeshType meshTypeValue{ PREPRO_BASE_NAMESPACE::PFFaceMeshType::EInvalidFaceMeshType };
    pfFaceMeshParameters->getFaceMeshSetting(2, faceIds, maxSizeValue, meshTypeValue);
    std::cout << "Max size of face mesh : " << maxSizeValue << std::endl;
    std::cout << "Mesh face type : " << static_cast<int>(meshTypeValue) << std::endl;

    pfFaceMeshParameters->setFaceMeshSetting(2, faceIds, 0.1066, PFFaceMeshType::EUnstructuredQuads);
    pfFaceMeshParameters->getFaceMeshSetting(2, faceIds, maxSizeValue, meshTypeValue);
    std::cout << "Max size of face mesh : " << maxSizeValue << std::endl;
    std::cout << "Mesh face type : " << static_cast<int>(meshTypeValue) << std::endl;

    int faceIds2[3] = { 25, 26, 27 };
    pfFaceMeshParameters->setFaceMeshSetting(3, faceIds2, 2.5678, PFFaceMeshType::EUnstructuredQuads);
    pfFaceMeshParameters->getFaceMeshSetting(3, faceIds2, maxSizeValue, meshTypeValue);
    std::cout << "Max size of face mesh : " << maxSizeValue << std::endl;
    std::cout << "Mesh face type : " << static_cast<int>(meshTypeValue) << std::endl;
}