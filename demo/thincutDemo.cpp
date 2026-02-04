// Revision: $Id: thincutDemo.cpp,v 1.0 2025/03/27 18:15:20 Leon Exp $
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
#include <mesh/pfLocalSizeParameters.h>
#include <mesh/pfInflationParameters.h>
#include <mesh/pfShrinkwrapParameters.h>
#include <mesh/pfMeshLeakPath.h>
#include <mesh/pfThinCutSetting.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"

void MeshDemo::thinCut(const std::string& examplePath)
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

    std::string filePath = examplePath + "mesh\\new_thincut.ppcf";
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
    
    PREPRO_MESH_NAMESPACE::PFThinCutSettings thinCutData;

    const char* name1 = "Setting1";
    int totalSideCount1 = 2;
    int groupCounts1[] = {1, 1};
    char* groupNames1[] = {"RearWheels", "road"};
    double criteria1 = 0.0012;
    PREPRO_MESH_NAMESPACE::PFThinCutSetting setting1(name1, totalSideCount1, groupCounts1, groupNames1, criteria1);
    pfMesh->addThinCutSetting(&setting1);
    pfMesh->getThinCutSettings(thinCutData);
    int testCount = thinCutData.getSettingCount();
    for (int i = 0; i < testCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFThinCutSetting* setting = thinCutData.getSetting(i);
        std::cout << "Name: " << setting->getName() << std::endl;
        std::cout << "Total Side Count: " << setting->getTotalSideCount() << std::endl;

        const int* groupCounts = setting->getGroupCountsInEachSide();
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            std::cout << "Group Count in Side " << j << ": " << groupCounts[j] << std::endl;
        }

        auto groupNames = setting->getGroupNamesInEachSide();
        int totalGroupNames = 0;
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            totalGroupNames += groupCounts[j];
        }
        for (int j = 0; j < totalGroupNames; ++j)
        {
            std::cout << "Group Name " << j << ": " << groupNames[j] << std::endl;
        }

        std::cout << "Criteria: " << setting->getCriteria() << std::endl;
        std::cout << "-------------------" << std::endl;
    }

    //test delete one setting
    pfMesh->deleteThinCutSetting("Setting1");
    pfMesh->getThinCutSettings(thinCutData);
    testCount = thinCutData.getSettingCount();
    std::cout << "#######              DELETE                 ###################" << std::endl;
    std::cout << "####### After deleting one thin cut setting -------------------" << std::endl;
    for (int i = 0; i < testCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFThinCutSetting* setting = thinCutData.getSetting(i);
        std::cout << "Name: " << setting->getName() << std::endl;
        std::cout << "Total Side Count: " << setting->getTotalSideCount() << std::endl;

        const int* groupCounts = setting->getGroupCountsInEachSide();
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            std::cout << "Group Count in Side " << j << ": " << groupCounts[j] << std::endl;
        }

        auto groupNames = setting->getGroupNamesInEachSide();
        int totalGroupNames = 0;
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            totalGroupNames += groupCounts[j];
        }
        for (int j = 0; j < totalGroupNames; ++j)
        {
            std::cout << "Group Name " << j << ": " << groupNames[j] << std::endl;
        }

        std::cout << "Criteria: " << setting->getCriteria() << std::endl;
        std::cout << "-------------------" << std::endl;
    }

    // add one new thin cut setting.
    const char* name2 = "Setting222";
    int totalSideCount2 = 2;
    int groupCounts2[] = { 1, 1 };
    char* groupNames2[] = { "RearWheels", "road" };
    double criteria2 = 0.0012;
    PREPRO_MESH_NAMESPACE::PFThinCutSetting setting2(name2, totalSideCount2, groupCounts2, groupNames2, criteria2);
    pfMesh->addThinCutSetting(&setting2);

    //test rename one setting
    pfMesh->renameThinCutSetting("Setting222", "RenamedSetting222");
    pfMesh->getThinCutSettings(thinCutData);
    testCount = thinCutData.getSettingCount();
    std::cout << "#######              RENAME         ###################" << std::endl;
    std::cout << "####### Rename one thin cut setting -------------------" << std::endl;
    for (int i = 0; i < testCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFThinCutSetting* setting = thinCutData.getSetting(i);
        std::cout << "Name: " << setting->getName() << std::endl;
        std::cout << "Total Side Count: " << setting->getTotalSideCount() << std::endl;

        const int* groupCounts = setting->getGroupCountsInEachSide();
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            std::cout << "Group Count in Side " << j << ": " << groupCounts[j] << std::endl;
        }

        auto groupNames = setting->getGroupNamesInEachSide();
        int totalGroupNames = 0;
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            totalGroupNames += groupCounts[j];
        }
        for (int j = 0; j < totalGroupNames; ++j)
        {
            std::cout << "Group Name " << j << ": " << groupNames[j] << std::endl;
        }

        std::cout << "Criteria: " << setting->getCriteria() << std::endl;
        std::cout << "-------------------" << std::endl;
    }

    // edit thin cut setting of "RenameSetting222", with some other setting parameters.
    const char* name3 = "RenamedSetting222";
    int totalSideCount3 = 3;
    int groupCounts3[] = { 1, 1, 1};
    char* groupNames3[] = { "RearWheels", "FrontWheels", "road" };
    double criteria3 = 0.001001112;
    PREPRO_MESH_NAMESPACE::PFThinCutSetting setting3(name3, totalSideCount3, groupCounts3, groupNames3, criteria3);
    pfMesh->editThinCutSetting(name3, &setting3);
    
    pfMesh->getThinCutSettings(thinCutData);
    testCount = thinCutData.getSettingCount();
    std::cout << "#######              EDIT         ###################" << std::endl;
    std::cout << "####### Edit one thin cut setting -------------------" << std::endl;
    for (int i = 0; i < testCount; ++i)
    {
        const PREPRO_MESH_NAMESPACE::PFThinCutSetting* setting = thinCutData.getSetting(i);
        std::cout << "Name: " << setting->getName() << std::endl;
        std::cout << "Total Side Count: " << setting->getTotalSideCount() << std::endl;

        const int* groupCounts = setting->getGroupCountsInEachSide();
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            std::cout << "Group Count in Side " << j << ": " << groupCounts[j] << std::endl;
        }

        auto groupNames = setting->getGroupNamesInEachSide();
        int totalGroupNames = 0;
        for (int j = 0; j < setting->getTotalSideCount(); ++j)
        {
            totalGroupNames += groupCounts[j];
        }
        for (int j = 0; j < totalGroupNames; ++j)
        {
            std::cout << "Group Name " << j << ": " << groupNames[j] << std::endl;
        }

        std::cout << "Criteria: " << setting->getCriteria() << std::endl;
        std::cout << "-------------------" << std::endl;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    DocumentDemo::closeDocument(&pfApplication, pfDocument, false);
}