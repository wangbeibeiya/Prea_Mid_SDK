// Revision: $Id: periodicFaceDemo.cpp,v 1.0 2025/04/02 15:40:20 Leon Exp $
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
#include <mesh/pfMeshData.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"

void MeshDemo::periodicFace(const std::string& path)
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

    std::string filePath = path + "mesh\\periodic.ppcf";
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

    PREPRO_MESH_NAMESPACE::PFPeriodicFaceSetting periodicFaceSetting;
    periodicFaceSetting.faceSource = 133;
    periodicFaceSetting.faceTarget = 134;
    periodicFaceSetting.periodicType = PREPRO_BASE_NAMESPACE::PFPeriodicFaceType::EPeriodicRotate;
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createPeriodicFace(periodicFaceSetting);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create periodic face is successful.\n";
    }
    else
    {
        std::cout << "Create periodic face failed.\n";
    }

    PREPRO_MESH_NAMESPACE::PFPeriodicFaceSettings allPeriodicFaceSettings;
    status = pfMesh->getAllPeriodicFaces(allPeriodicFaceSettings);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EError)
    {
        std::cout << "Get all periodic face failed.\n";
        return;
    }

    unsigned int periodicFaceCount = allPeriodicFaceSettings.getSettingCount();
    for (unsigned int i = 0; i < periodicFaceCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFPeriodicFaceSetting* faceSetting = allPeriodicFaceSettings.getSetting(i);
        std::cout << "Face is : [" << faceSetting->faceSource << ", " << faceSetting->faceTarget << "]\n";
        std::cout << "Face periodic type is : " << static_cast<int>(faceSetting->periodicType) << std::endl;
    }

    status = pfMesh->deletePeriodicFace(133, 134);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete periodic face is successful.\n";
    }
    else
    {
        std::cout << "Delete periodic face failed.\n";
    }
    
    // check whether it is deleted from all periodic face settings.
    status = pfMesh->getAllPeriodicFaces(allPeriodicFaceSettings);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EError)
    {
        std::cout << "Get all periodic face failed.\n";
        return;
    }

    periodicFaceCount = allPeriodicFaceSettings.getSettingCount();
    std::cout << "After Delete periodic face of : [133, 134], the remaining periodic face settings count is: "
        << periodicFaceCount << std::endl;

    for (unsigned int i = 0; i < periodicFaceCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFPeriodicFaceSetting* faceSetting = allPeriodicFaceSettings.getSetting(i);
        std::cout << "Face is : [" << faceSetting->faceSource << ", " << faceSetting->faceTarget << "]\n";
        std::cout << "Face periodic type is : " << static_cast<int>(faceSetting->periodicType) << std::endl;
    }

    // ##################################################
    // add again and delete all, then check.
    std::cout << "\n\n##################  Add again ##################### \n\n";
    status = pfMesh->createPeriodicFace(periodicFaceSetting);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create periodic face is successful.\n";
    }
    else
    {
        std::cout << "Create periodic face failed.\n";
    }

    status = pfMesh->getAllPeriodicFaces(allPeriodicFaceSettings);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EError)
    {
        std::cout << "Get all periodic face failed.\n";
        return;
    }

    periodicFaceCount = allPeriodicFaceSettings.getSettingCount();
    for (unsigned int i = 0; i < periodicFaceCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFPeriodicFaceSetting* faceSetting = allPeriodicFaceSettings.getSetting(i);
        std::cout << "Face is : [" << faceSetting->faceSource << ", " << faceSetting->faceTarget << "]\n";
        std::cout << "Face periodic type is : " << static_cast<int>(faceSetting->periodicType) << std::endl;
    }

    status = pfMesh->deleteAllPeriodicFaces();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete all periodic faces is successful.\n";
    }
    else
    {
        std::cout << "Delete all periodic faces failed.\n";
    }

    // check whether it is deleted from all periodic face settings.
    status = pfMesh->getAllPeriodicFaces(allPeriodicFaceSettings);
    periodicFaceCount = allPeriodicFaceSettings.getSettingCount();
    std::cout << "After Delete all periodic faces, the remaining periodic face settings count is: "
        << periodicFaceCount << std::endl;

    for (unsigned int i = 0; i < periodicFaceCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFPeriodicFaceSetting* faceSetting = allPeriodicFaceSettings.getSetting(i);
        std::cout << "Face is : [" << faceSetting->faceSource << ", " << faceSetting->faceTarget << "]\n";
        std::cout << "Face periodic type is : " << static_cast<int>(faceSetting->periodicType) << std::endl;
    }
}