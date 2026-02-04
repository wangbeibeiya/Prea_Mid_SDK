// Revision: $Id: smoothVolumeMeshDemo.cpp,v 1.0 2025/04/11 13:35:20 Leon Exp $
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
#include <mesh/pfMeshBuilder.h>
#include <mesh/pfMeshData.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"

void MeshDemo::smoothVolumeMesh(const std::string& path)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (nullptr != geomertyBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(geomertyBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }
    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr != meshBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    std::string filePath = path + "mesh\\convert_polyhedral_mesh.ppcf";
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

    PREPRO_MESH_NAMESPACE::PFVolumeMeshSmoothSetting smoothSetting;
    smoothSetting.mIterations = 4;
    smoothSetting.mMaxLimitForQuality = 0.2;
    smoothSetting.mSmoothSurface = true;

    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->smoothVolumeMesh(smoothSetting);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Smooth volume mesh failed." << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    DocumentDemo::closeDocument(&pfApplication, pfDocument, false);
}