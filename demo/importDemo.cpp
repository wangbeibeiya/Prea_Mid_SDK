// Revision: $Id: importDemo.cpp,v 1.0 2025/02/10 16:55:04 xiaming Exp $
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
#include "importDemo.h"
#include "RenderDemo.h"
#include "commonMethods.h"

#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <geometry/pfGeometryBuilder.h>
#include <mesh/pfMesh.h>
#include "mesh/pfMeshBuilder.h"
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <geometry/pfSTLImportOptions.h>

#include <iostream>
#include <chrono>

void ImportDemo::importDemo(const std::string& examplePath)
{
    std::cout << "##### Import Demo: " << std::endl;
    std::cout << "#####   1 - import geometry." << std::endl;
    std::cout << "#####   2 - check configure of cad import for group mode." << std::endl;
    std::cout << "#####   3 - set import STL options." << std::endl;
    std::cout << "input number ->  " << std::endl;

    int type = 0;
    std::cin >> type;

    switch (type)
    {
    case 1:
        importGeometry(examplePath);
        break;
    case 2:
    default:
        checkSetImportGroupMode(examplePath);
        break;
    case 3:
        setImportSTLOptions(examplePath);
        break;
    }
}

void ImportDemo::importGeometry(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        printf_s("Current Application has no geometry privilege");
        return;
    }

    // set import option for group mode.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromFileName);

    // import first geometry 
    std::string geometryOnePath = examplePath + "import\\HeatSink_cf.stl";
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->importGeometry(geometryOnePath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryOnePath << std::endl;
        return;
    }

    // replace first geometry
    std::string geometryTwoPath = examplePath + "import\\j_shape.stl";
    status = geometry->importGeometry(geometryTwoPath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryTwoPath << std::endl;
        return;
    }

    // append a geometry
    std::string geometryThreePath = examplePath + "import\\pipetee.stp";
    status = geometry->importGeometry(geometryThreePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryThreePath << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFData geometryData;
    geometry->getAllData(geometryData);
    RenderDemo::show(geometryData, VisualizationType::EGeomerty);
}

void ImportDemo::importPostProcessResult(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = PREPRO_BASE_NAMESPACE::PFStatus::EError;
    /*!
    * 1.new document,add post process environment
    */
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder* postProcessBuilder = PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder::getInstance();
    if (nullptr != postProcessBuilder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(postProcessBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess = dynamic_cast<PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess*>(pfDocument->getPostProcessEnvironment());
    if (nullptr == postProcess)
    {
        return;
    }
    /*!
    * 2.load mesh and result.
    */
    std::chrono::steady_clock::time_point start, end;
    std::chrono::duration<double> elapsed;
    std::cout << "Load mesh and result." << std::endl;
    std::string resultPath = examplePath + "postProcess\\Pipetee-100.cgns";
    start = std::chrono::high_resolution_clock::now();
    status = postProcess->loadResult(resultPath.c_str());
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        std::cout << "Use time:" << elapsed.count() << "seconds." << std::endl;
        std::cout << "Result file is loaded successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to load result file." << std::endl;
    }    
    /*!
    * 4.only load result, not import mesh.
    */
    std::cout << "Load result." << std::endl;
    resultPath = examplePath + "postProcess\\Pipetee-273.cgns";
    start = std::chrono::high_resolution_clock::now();
    status = postProcess->loadResult(resultPath.c_str());
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        std::cout << "Use time:" << elapsed.count() << "seconds." << std::endl;
        std::cout << "Result file is loaded successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to load result file." << std::endl;
    }
}

void ImportDemo::setImportSTLOptions(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            std::cout << "No PERA SIM license\n";
            return;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        std::cout << "Current Application has no geometry privilege\n";
        return;
    }

    // set Import STL optios.
    PREPRO_GEOMETRY_NAMESPACE::PFSTLImportOptions* stlImportOptions = geometry->getSTLImportOptions();
    if (stlImportOptions == nullptr)
    {
        std::cout << "STL import options read error.\n";
        return;
    }

    stlImportOptions->setFeatureAngle(20.0);
    stlImportOptions->setNeedMergeVertexAcrossSolid(false);
    stlImportOptions->setMergeVertexTolerance(0.00001);
    stlImportOptions->setScale(1.5);
    // ##############################
    // set import STL options: group mode = group from filename.
    stlImportOptions->setGroupMode(PREPRO_BASE_NAMESPACE::PFImportSTLGroupMode::EFromFileName);

    std::string geometryPath = examplePath + "import\\HeatSink_cf.stl";
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->importGeometry(geometryPath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryPath << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFData geometryData;
    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "Setting STL import mode => group mode to [File name].\n\n";
    unsigned groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    PREPRO_BASE_NAMESPACE::PFGroup** groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << "STL Option - Feature Angle: " << stlImportOptions->getFeatureAngle() << std::endl;
    std::cout << "STL Option - Merge vertex across solid(bool): " << std::string(stlImportOptions->isNeedMergeVertexAcrossSolid() ? "true" : "false") << std::endl;
    std::cout << "STL Option - Merge vertex tolerance: " << stlImportOptions->getMergeVertexTolerance() << std::endl;
    std::cout << "STL Option - Scale: " << stlImportOptions->getScale() << std::endl;

    std::cout << std::endl;

    // ##############################
    // set import STL options: group mode = group from surfname.
    stlImportOptions->setGroupMode(PREPRO_BASE_NAMESPACE::PFImportSTLGroupMode::EFromSurfaceName);
    status = geometry->importGeometry(geometryPath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryPath << std::endl;
        return;
    }

    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "Setting STL import mode => group mode to [Surface name].\n\n";
    groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;


    // ##############################
    // set import STL options: group mode = one group for all files.
    stlImportOptions->setGroupMode(PREPRO_BASE_NAMESPACE::PFImportSTLGroupMode::EOneGroupForAllFiles);
    status = geometry->importGeometry(geometryPath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryPath << std::endl;
        return;
    }

    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "Setting STL import mode => group mode to [One group for all files].\n\n";
    groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;

    // ##############################
    // set import STL options: group mode = group from surface with file name.
    stlImportOptions->setGroupMode(PREPRO_BASE_NAMESPACE::PFImportSTLGroupMode::EFromSurfaceWithFileName);
    status = geometry->importGeometry(geometryPath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryPath << std::endl;
        return;
    }

    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "Setting STL import mode => group mode to [Surface with file name].\n\n";
    groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;
    RenderDemo::show(geometryData, VisualizationType::EGeomerty);
}

void ImportDemo::checkSetImportGroupMode(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        printf_s("Current Application has no geometry privilege");
        return;
    }

    ///====================== set group mode to File name. ==========================

    // set import option for group mode.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromFileName);

    //just test whether the group mode is configured successfully.
    PREPRO_BASE_NAMESPACE::PFImportGroupMode groupMode = geometry->getCADImportGroupMode();
    if (groupMode != PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromFileName)
    {
        std::cout << "Warning: Config import option for group mode failed.\n";
        std::cout << "==> We will use default config for group mode: group from file name.\n";
    }
    else
    {
        std::cout << "Hint: Config import option for group mode is successful.\n";
        std::cout << "==> Current import option for group mode is : " << (int)groupMode << std::endl;
    }
    
    std::string geometryThreePath = examplePath + "import\\pipetee.stp";
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->importGeometry(geometryThreePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryThreePath << std::endl;
        return;
    }

    // check data using different group mode.
    PREPRO_BASE_NAMESPACE::PFData geometryData;
    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "Setting group mode to File name.\n\n";
    unsigned groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    //just test whether the group mode is configured successfully.
    // this should display as the following:
    /*
        Setting group mode to File name.

        Group size is : 3
        (+) .double edges
        (+).nodes
        (+) pipetee
    */
    //////////////////////////////////////

    PREPRO_BASE_NAMESPACE::PFGroup** groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;



    ///====================== set group mode to Surface name. ==========================

    // set import option for group mode = from surface name.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromSurfaceName);


    groupMode = geometry->getCADImportGroupMode();
    if (groupMode != PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromSurfaceName)
    {
        std::cout << "Warning: Config import option for group mode failed.\n";
        std::cout << "==> We will use default config for group mode: group from file name.\n";
    }

    status = geometry->importGeometry(geometryThreePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryThreePath << std::endl;
        return;
    }

    // check data using different group mode.
    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "After setting group mode to Surface name.\n\n";
    groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    // this should display as the following:
    /*
        After setting group mode to Surface name.

        Group size is: 9
        (+) P_0194
        (+) .double edges
        (+) .nodes
        (+) P_0252
        (+) P_0185
        (+) P_0188
        (+) P_0197
        (+) P_0191
        (+) P_0200
    */
    //////////////////////////////////////

    groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;


    ///====================== set group mode to Body name. ==========================

    // set import option for group mode = from surface name.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromBodyName);
    groupMode = geometry->getCADImportGroupMode();
    if (groupMode != PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromBodyName)
    {
        std::cout << "Warning: Config import option for group mode failed.\n";
        std::cout << "==> We will use default config for group mode: group from file name.\n";
    }

    status = geometry->importGeometry(geometryThreePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import Geometry failed: => " << geometryThreePath << std::endl;
        return;
    }

    // check data using different group mode.
    geometry->getAllData(geometryData);

    std::cout << "\n\n==========================================================\n";
    std::cout << "After setting group mode to Body name.\n\n";
    groupSize = geometryData.getGroupSize();
    std::cout << "Group size is: " << groupSize << std::endl;

    // this should display as the following:
    /*
        After setting group mode to Body name.

        Group size is: 4
        (+) .double edges
        (+) .nodes
        (+) pipetee
        (+) Volume1
    */
    //////////////////////////////////////

    groups = geometryData.getGroups();
    for (unsigned i = 0; i < groupSize; ++i)
    {
        std::string groupName = groups[i]->getName();
        std::cout << "(+) " << groupName << "\n";
    }
    std::cout << std::endl;
}