// Revision: $Id: edgeMeshParametersDemo.cpp,v 1.0 2025/04/02 10:00:20 Leon Exp $
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
#include <mesh/pfEdgeMeshParameters.h>
#include <mesh/pfMeshData.h>
#include <mesh/pfMeshBuilder.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>

void MeshDemo::edgeMeshParameter(const std::string& examplePath)
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

    PREPRO_MESH_NAMESPACE::PFEdgeMeshSetting edgeMeshSetting;
    edgeMeshSetting.mMaxSize = 0.015;
    edgeMeshSetting.mNodeCount = 2;
    edgeMeshSetting.mSpacing1 = 0.003;
    edgeMeshSetting.mGrowthRatio1 = 1.12;
    edgeMeshSetting.mSpacing2 = 0.003;
    edgeMeshSetting.mGrowthRatio2 = 1.2;
    
    PREPRO_MESH_NAMESPACE::PFEdgeMeshParameters* edgeMeshParameters = pfMesh->getEdgeMeshParameters();
    int edgeIds[2] = { 14, 15 };
    edgeMeshParameters->setEdgeMeshSetting(2, edgeIds, edgeMeshSetting);

    PREPRO_MESH_NAMESPACE::PFEdgeMeshSetting edgeMeshSettingResult;
    edgeMeshParameters->getEdgeMeshSetting(2, edgeIds, edgeMeshSettingResult);
    std::cout << "Max size: " << edgeMeshSettingResult.mMaxSize << std::endl
        << "Node count: " << edgeMeshSettingResult.mNodeCount << std::endl
        << "Spacing 1: " << edgeMeshSettingResult.mSpacing1 << std::endl
        << "Growth ratio 1: " << edgeMeshSettingResult.mGrowthRatio1 << std::endl
        << "Spacing 2: " << edgeMeshSettingResult.mSpacing2 << std::endl
        << "Growth ratio 2: " << edgeMeshSettingResult.mGrowthRatio2 << std::endl
        << std::endl;
}