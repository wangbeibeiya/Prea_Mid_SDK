// Revision: $Id: meshDemo.cpp,v 1.0 2025/02/21 13:15:20 liujing Exp $
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
#include <unordered_map>
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
#include <mesh/pfBoxDensitySetting.h>
#include <mesh/pfSphereDensitySetting.h>
#include <mesh/pfCylinderDensitySetting.h>
#include <mesh/pfDensitySettings.h>
#include <mesh/pfBoundaryConditions.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"
#include "surfaceMeshEditDemo.h"
#include "fileMonitor.h"

void MeshDemo::meshDemo(const std::string& examplePath)
{
    int type = 1;
    std::cout << "Select one mesh demo type: " << std::endl;
    std::cout << "#### 1. Surface Mesh. " << std::endl;
    std::cout << "#### 2. Volume Mesh by Geometry. " << std::endl;
    std::cout << "#### 3. Export Mesh. " << std::endl;
    std::cout << "#### 4. Create wrap Mesh. " << std::endl;
    std::cout << "#### 5. ReMesh(surface mesh). " << std::endl;
    std::cout << "#### 6. Volume Mesh by Surface Mesh. " << std::endl;
    std::cout << "#### 7. Compute mesh leak dection. " << std::endl;
    std::cout << "#### 8. Density. " << std::endl;
    std::cout << "#### 9. Thin cut. \n";
    std::cout << "#### 10. Face Mesh Parameters. \n";
    std::cout << "#### 11. Edge Mesh Parameters. \n";
    std::cout << "#### 12. Surface Mesh Edit. \n";
    std::cout << "#### 13. Periodic face. \n";
    std::cout << "#### 14. Convert to Polyhedral mesh. \n";
    std::cout << "#### 15. Smooth Volume Mesh. \n";
    std::cout << "#### 16. Mesh Quality demo. \n";
    std::cout << "#### 17. Inflation setting and volume mesh with inflation on. \n";
    std::cout << "#### 18. Convert geometry to mesh. \n";
    std::cout << "#### 19. Cut plane. \n";
    std::cout << "#### 20. import mesh. \n";
    std::cin >> type;
    if (type == 3)
    {
        exportMesh(examplePath);
        return;
    }
    else if (type == 4)
    {
        createShrinkWrapMesh(examplePath);
        return;
    }
    else if (type == 5)
    {
        Remesh(examplePath);
        return;
    }
    else if (type == 6)
    {
        createVolumeMeshBySurfaceMesh(examplePath);
        return;
    }
    else if (type == 7)
    {
        computeLeakDetection(examplePath);
        return;
    }
    else if (type == 8)
    {
        DensityDemo(examplePath);
        return;
    }
    else if (type == 9)
    {
        thinCut(examplePath);
        return;
    }
    else if (type == 10)
    {
        faceMeshParametersDemo(examplePath);
        return;
    }
    else if (type == 11)
    {
        edgeMeshParameter(examplePath);
        return;
    }
    else if (type == 12)
    {
        SurfaceMeshEditDemo::surfaceMeshEditDemo(examplePath);
        return;
    }
    else if (type == 13)
    {
        periodicFace(examplePath);
        return;
    }
    else if (type == 14)
    {
        convertToPolyhedralMesh(examplePath);
        return;
    }
    else if (type == 15)
    {
        smoothVolumeMesh(examplePath);
        return;
    }
    else if (type == 16)
    {
        meshQualityDemo(examplePath);
        return;
    }
    else if (type == 17)
    {
        inflationSettingDemo(examplePath);
        return;
    }
    else if (type == 18)
    {
        convertGeometryToMesh(examplePath);
        return;
    }
    else if (type == 19)
    {
        cutPlaneDemo(examplePath);
        return;
    }
    else if (type == 20)
    {
        importMesh(examplePath);
        return;
    }
    /*!
        1.new document,add geometry and mesh environment
    */
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }

    /*!
        2.import geometry
    */
    std::string filePath = examplePath + "mesh\\pipetee.stp";
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        return;
    }
    if (pfGeometry->importGeometry(filePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import geometry." << std::endl;
        return;
    }
    /*!
        3.set parameters of mesh
    */
    setMeshGlobalParameters(pfDocument);

    setMeshLocalSizeParameters(pfDocument);

    // shrink wrap parameters.
    setShrinkwrapParameters(pfDocument);

    /*!
        4.create surface mesh
    */
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        return;
    }

    if (type == 1)
    {
        // We provide two approaches for data communication between client and library.
        // one is using file mapping, this has lower latency since it has no file IO consumption.
        // the other is using reading file directly, it will open the log file and get the data update
        // after the library writing a stream of data into the physical storage.
        int logtype = 2;
        std::cout << "Select log message update type: " << std::endl;
        std::cout << "#### 1. Use file mapping. " << std::endl;
        std::cout << "#### 2. Use log file reading. " << std::endl;
        std::cin >> logtype;

        const char* logfileName = "D:\\tmp\\api_log\\20250521\\11\\22\\sdklog.txt";
        pfDocument->setLogFile(logfileName);

        if (logtype == 1)
        {
            const char* mapName = "sdklogdata";
            const char* eventName = "sdklogdataevent";
            pfDocument->setLogMappingInfo(mapName, eventName);
            FileMappingMonitor monitor(mapName, eventName);
            monitor.start();

            if (createSurfaceMesh(pfMesh) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                return;
            }
        }
        else if (logtype == 2)
        {
            LogMonitor monitor(logfileName, [](const std::string& new_line) {
                std::cout << "[file_monitor]: " << new_line << std::endl;
                });
            monitor.start();

            if (createSurfaceMesh(pfMesh) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                return;
            }
        }
    }
    else if (type == 2)
    {
        // first we try to create volume mesh by geometry without inflation directly, no polyhedron either.
        // if it returned with status of PFStatus::ENoGeometryVolume,
        // we need to find volume, then try create volume by geometry again.
        PREPRO_BASE_NAMESPACE::PFStatus volumeMeshStatus = createVolumeMeshByGeometryNoInflationNoPolyhedron(pfMesh);
        if (volumeMeshStatus == PREPRO_BASE_NAMESPACE::PFStatus::ENoGeometryVolume)
        {
            // find volume.
            if (pfGeometry->findVolumes() != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                std::cout << "Find volume is not successful before creating volume mesh." << std::endl;
                return;
            }
            
            if (createVolumeMeshByGeometryNoInflationNoPolyhedron(pfMesh) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                std::cout << "Volume mesh by Geometry(no inflation/no polyhedron) is failed." << std::endl;
                return;
            }
        }
        else  // it has volume.
        {
            if (volumeMeshStatus != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
            {
                std::cout << "Volume mesh by Geometry is failed." << std::endl;
                return;
            }
        }
    }
    else
    {
        return;
    }

    /*!
        5.get mesh data
    */
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);

    // test mesh volume data.
    std::cout << "Mesh Group Size is : " << pfData.getGroupSize() << std::endl;
    std::cout << "Mesh Volume Size is : " << pfData.getVolumeSize() << std::endl;

    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void MeshDemo::inflationSettingDemo(const std::string& examplePath)
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

    std::string filePath = examplePath + "testInflation.ppcf";
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
        std::cout << "!!! Mesh is null." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFInflationParameters* inflationParameters = pfMesh->getInflationParameters();
    if (nullptr == inflationParameters)
    {
        return;
    }

    // first must set inflation on before we use inflation setting.
    inflationParameters->setInflationOn("Volume1", "Share", true);

    PREPRO_BASE_NAMESPACE::PFStatus error = inflationParameters->setInitialHeight("Volume1", "Share", 0.00866462);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set initial height. Check parameter.\n";
    }

    error = inflationParameters->setLayerNumber("Volume1", "Share", 3);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set layer number. Check parameter.\n";
    }

    error = inflationParameters->setHeightRatio("Volume1", "Share", 1.25);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set height ratio. Check parameter.\n";
    }

    // get the values just set above.
    std::cout << "Initial Height: " << inflationParameters->getInitialHeight("Volume1", "Share") << std::endl;
    std::cout << "Layer number: " << inflationParameters->getLayerNumber("Volume1", "Share") << std::endl;
    std::cout << "Height Ratio: " << inflationParameters->getHeightRatio("Volume1", "Share") << std::endl;

    std::cout << "Before setting Max Total Height, its value: " << inflationParameters->getMaxTotalHeight("Volume1", "Share") << std::endl;


    // create volume mesh. (with inflation on)
    error = pfMesh->createVolumeMeshByGeometry(true, false);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create volume mesh by geometry failed.\n";
        return;
    }
    else
    {
        std::cout << "Create volume mesh by geometry is successful.\n";
    }


    PREPRO_BASE_NAMESPACE::PFData meshData;
    pfMesh->getAllData(meshData);
    RenderDemo::show(meshData, VisualizationType::EMesh);
}

 /*!
     \brief set the global parameters of mesh
 */
void MeshDemo::setMeshGlobalParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument)
{
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }
    PREPRO_MESH_NAMESPACE::PFGlobalParameters* globalParameters = pfMesh->getGlobalParameters();
    if (nullptr == globalParameters)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus error = globalParameters->setSize(0.01, 0.1);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set globalParameters.Check globalParameters\n";
        return;
    }
    error = globalParameters->setGrowthRate(1.2);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setCurvatureNormalAngle(30);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setProximityEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setCellsPerGap(3);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setMinimumGapSize(0.01);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setAllTrianglesEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setMeshQualityOptimizationEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setInflationMinimumQuality(0.05);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setInflationSeparatingAngle(60);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setInflationMaximumHeightBaseRatio(1.3);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setInflationAutoReductionEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setKillLowQualityElementsEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setKillWorseThanThreshold(0.01);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setKillWedgeQualityElementsEnabled(true);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = globalParameters->setKillSharperThanThreshold(10);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    //
    std::cout << "Minimum Size: " << globalParameters->getMinimumSize() << std::endl;
    std::cout << "Maximum Size: " << globalParameters->getMaximumSize() << std::endl;
    std::cout << "Growth Rate: " << globalParameters->getGrowthRate() << std::endl;
    std::cout << "Curvature Normal Angle: " << globalParameters->getCurvatureNormalAngle() << std::endl;
    std::cout << "Proximity Enabled: " << globalParameters->isProximityEnabled() << std::endl;
    std::cout << "Cells Per Gap: " << globalParameters->getCellsPerGap() << std::endl;
    std::cout << "Minimum Gap Size: " << globalParameters->getMinimumGapSize() << std::endl;
    std::cout << "All Triangles Enabled: " << globalParameters->isAllTrianglesEnabled() << std::endl;
    std::cout << "Mesh Quality Optimization Enabled: " << globalParameters->isMeshQualityOptimizationEnabled() << std::endl;
    std::cout << "Inflation Minimum Quality: " << globalParameters->getInflationMinimumQuality() << std::endl;
    std::cout << "Inflation Separating Angle: " << globalParameters->getInflationSeparatingAngle() << std::endl;
    std::cout << "Inflation Maximum Height Base Ratio: " << globalParameters->getInflationMaximumHeightBaseRatio() << std::endl;
    std::cout << "Inflation Auto Reduction Enabled: " << globalParameters->isInflationAutoReductionEnabled() << std::endl;
    std::cout << "Kill Low Quality Elements Enabled: " << globalParameters->isKillLowQualityElementsEnabled() << std::endl;
    std::cout << "Kill Worse Than Threshold: " << globalParameters->getKillWorseThanThreshold() << std::endl;
    std::cout << "Kill Wedge Quality Elements Enabled: " << globalParameters->isKillWedgeQualityElementsEnabled() << std::endl;
    std::cout << "Kill sharper than threshold: " << globalParameters->getKillSharperThanThreshold() << std::endl;

}

/*!
    \brief create surface mesh
*/
PREPRO_BASE_NAMESPACE::PFStatus MeshDemo::createSurfaceMesh(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh)
{
    /*!
        create surface mesh
    */
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createSurfaceMesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Surface mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Surface mesh creation failed." << std::endl;
    }
    return status;
}

void MeshDemo::setMeshLocalSizeParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument)
{
    if (!pfDocument)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfGeometryEnvironment = pfDocument->getGeometryEnvironment();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfGeometryEnvironment);
    if (nullptr == pfGeometry)
    {
        return;
    }
    PREPRO_MESH_NAMESPACE::PFLocalSizeParameters* localSizeParameters = pfMesh->getLocalSizeParameters();
    if (nullptr == localSizeParameters)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFData geometryData;
    pfGeometry->getAllData(geometryData);

    auto groupNum = geometryData.getGroupSize();
    if (groupNum <= 0)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFGroup** groups = geometryData.getGroups();
    if (!groups)
    {
        return;
    }
    std::string groupName;
    for (auto i=0;i< groupNum;i++)
    {
        std::string tempStr = std::string(groups[i]->getName());
        if (tempStr.find("node") == std::string::npos && tempStr.find("edge") == std::string::npos)
        {
            groupName = tempStr;
            break;
        }
    }
    if (groupName.empty())
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus error = localSizeParameters->setMinimunAndMaximunSize(groupName.c_str(), 0.00364006, 0.0182003);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set local size parameters.Check local size parameters\n";
        return;
    }
    error = localSizeParameters->setGrowthRate(groupName.c_str(), 1.2);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }
    error = localSizeParameters->setCurvatureNormalAngle(groupName.c_str(), 20);
    if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set parameter.Check parameter\n";
        return;
    }

    //
    std::cout << "Minimum Size: " << localSizeParameters->getMinimumSize(groupName.c_str()) << std::endl;
    std::cout << "Maximum Size: " << localSizeParameters->getMaximumSize(groupName.c_str()) << std::endl;
    std::cout << "Growth Rate: " << localSizeParameters->getGrowthRate(groupName.c_str()) << std::endl;
    std::cout << "Curvature Normal Angle: " << localSizeParameters->getCurvatureNormalAngle(groupName.c_str()) << std::endl;
}

/*!
    \brief export mesh file
*/
void MeshDemo::exportMesh(const std::string& path)
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }

    // use this file as the export source. do the export to the mesh file of .msh/.stl/.cgns format.
    std::string meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\test_pipetee_with_mesh.ppcf";
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file is OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
        return;
    }

    // export to .msh/.stl/.cgns mesh file.
    std::string exportSTLFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\exportmesh_pipetee.stl";
    status = pfMesh->exportSTLMesh(exportSTLFilePath.c_str());
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Exoprt to STL file is OK:" << exportSTLFilePath << std::endl;
    }
    else
    {
        std::cout << "Exoprt to STL file failed:" << exportSTLFilePath << std::endl;
    }

    // export to .msh file.
    std::string exportFluentFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\exportmesh_pipetee.msh";
    status = pfMesh->exportFluentMesh(exportFluentFilePath.c_str());
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Exoprt to Fluent mesh file is OK:" << exportFluentFilePath << std::endl;
    }
    else
    {
        std::cout << "Exoprt to Fluent mesh file failed:" << exportFluentFilePath << std::endl;
    }

    // export to .cgns file.
    std::string exportCGNSFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\exportmesh_pipetee.cgns";

    PREPRO_MESH_NAMESPACE::PFBoundaryConditions boundaryConditions;
    std::unordered_map<std::string, unsigned int> boundaryConditionsMap{ {"Èë¿Ú",9}, {"outlet",13},{"inlet",9},{"wall",20} };
    boundaryConditions.setGroupCount(boundaryConditionsMap.size());
    int index = 0;
    for (auto& itor = boundaryConditionsMap.cbegin(); itor != boundaryConditionsMap.cend();itor++)
    {
        boundaryConditions.setGroupNameAndType(index++, itor->first.c_str(), itor->second);
    }

    status = pfMesh->exportCGNSMesh(exportCGNSFilePath.c_str(), boundaryConditions);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Exoprt to CGNS file is OK:" << exportCGNSFilePath << std::endl;
    }
    else
    {
        std::cout << "Exoprt to CGNS file failed:" << exportCGNSFilePath << std::endl;
    }

    PREPRO_BASE_NAMESPACE::PFData meshData;
    status = pfMesh->getAllData(meshData);

    RenderDemo::show(meshData, VisualizationType::EMesh);
}

/*!
    \brief import mesh file
*/
void MeshDemo::importMesh(const std::string& path)
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }
    std::string meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\Pipetee.ppcf";
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
        return;
    }

    meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\Pump.ppcf";
    status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
    }

    meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\HECHT-POLY-cgns-HDF5.cgns";
    status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
    }

    meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\bVolume.msh";
    status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
    }

    PREPRO_BASE_NAMESPACE::PFData meshData;
    status = pfMesh->getAllData(meshData);

    RenderDemo::show(meshData, VisualizationType::EMesh);
}

/*!
    \brief Create volume mesh based on geometry
*/
PREPRO_BASE_NAMESPACE::PFStatus MeshDemo::createVolumeMeshByGeometryNoInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh)
{
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // we create volume mesh only. Without inflation and generating polyhedron.
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createVolumeMeshByGeometry(false, false);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Volume mesh by Geometry creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Volume mesh by Geometry creation failed." << std::endl;
    }
    return status;
}

/*!
    \brief set the shrink wrap parameters of mesh
*/
void MeshDemo::setShrinkwrapParameters(PREPRO_BASE_NAMESPACE::PFDocument* pfDocument)
{
    if (!pfDocument)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfGeometryEnvironment = pfDocument->getGeometryEnvironment();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfGeometryEnvironment);
    if (nullptr == pfGeometry)
    {
        return;
    }
    PREPRO_MESH_NAMESPACE::PFShrinkwrapParameters* shrinkwrapParameters = pfMesh->getShrinkwrapParameters();
    if (nullptr == shrinkwrapParameters)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus statusCode = shrinkwrapParameters->setGapCloseTolerance(0.02f);
    statusCode = shrinkwrapParameters->setGapCloseRefinement(1);
    statusCode = shrinkwrapParameters->setScaleDownFactor(0.9f);
    statusCode = shrinkwrapParameters->setExternalMethod();

    //check external method result of shrink wrap.
    PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_External)
    {
        std::cout << "Shrink wrap set External Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set External Method is successful." << std::endl;
    }


    /// ****************************************************************************************************
    //check seed point method result of shrink wrap.
    statusCode = shrinkwrapParameters->setSeedPointMethod(2.0123f, 3.0234f, 4.0345f);
    method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_SeedPoint)
    {
        std::cout << "Shrink wrap set Seedpoint Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set Seedpoint Method is successful." << std::endl;
    }
    
    //check seed point value.
    float seedPoint[3];
    shrinkwrapParameters->getSeedPointValue(&seedPoint[0]);
    if (std::abs(seedPoint[0] - 2.0123f) < std::numeric_limits<float>::epsilon()
        && std::abs(seedPoint[1] - 3.0234f) < std::numeric_limits<float>::epsilon()
        && std::abs(seedPoint[2] - 4.0345f) < std::numeric_limits<float>::epsilon())
    {
        std::cout << "Shrink wrap seed point value is correct." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap seed point value is wrong." << std::endl;
    }

    
    /// ****************************************************************************************************
    //check Largest method result of shrink wrap.
    statusCode = shrinkwrapParameters->setLargestMethod();
    method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_Largest)
    {
        std::cout << "Shrink wrap set Largest Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set Largest Method is successful." << std::endl;
    }


    /// ****************************************************************************************************
    //check Nth Largest method result of shrink wrap.
    statusCode = shrinkwrapParameters->setNthLargestMethod(4);
    method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_Nth_Largest)
    {
        std::cout << "Shrink wrap set Nth Largest Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set Nth Largest Method is successful." << std::endl;
    }

    //check shrink wrap volume value.
    int wrapValue = shrinkwrapParameters->getNthLargestValue();
    if (wrapValue != 4)
    {
        std::cout << "Shrink wrap volume value is wrong." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap volume value is correct." << std::endl;
    }


    /// ****************************************************************************************************
    //check First to Nth Largest method result of shrink wrap.
    statusCode = shrinkwrapParameters->setFirstToNthLargestMethod(6);
    method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_FirstToNthLargest)
    {
        std::cout << "Shrink wrap set First to Nth Largest Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set First to Nth Largest Method is successful." << std::endl;
    }

    //check shrink wrap volume value.
    wrapValue = shrinkwrapParameters->getFirstToNthLargestValue();
    if (wrapValue != 6)
    {
        std::cout << "Shrink wrap volume value is wrong." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap volume value is correct." << std::endl;
    }


    /// ****************************************************************************************************
    //check small volume shreshold method result of shrink wrap.
    statusCode = shrinkwrapParameters->setSmallVolumeThresholdMethod(0.78965f);
    method = shrinkwrapParameters->getShrinkWrapVolumeMethod();
    std::cout << "**************************************************************\n";
    if (method != PREPRO_BASE_NAMESPACE::PFWrapMeshVolumeMethod::EVolumeMethod_LargerThanThreshold)
    {
        std::cout << "Shrink wrap set First to Small Volume Threshold Method error." << std::endl;
    }
    else
    {
        std::cout << "Shrink wrap set First to Small Volume Threshold Method is successful." << std::endl;
    }

    // get shreshold value.
    float thresholdValue = shrinkwrapParameters->getSmallVolumeThresholdValue();
    std::cout << "Shrink wrap volume threshold value is: " << thresholdValue << std::endl;


    /// ****************************************************************************************************
    //check other settings of shrink wrap.
    statusCode = shrinkwrapParameters->setKeepInternalShells(true);
    statusCode = shrinkwrapParameters->setDeleteIsolatedVolume(true);
    statusCode = shrinkwrapParameters->setMaxElementNumber(2);
    int maxElementValue;
    // here we should succeed with the max element number setting.
    if (statusCode == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        maxElementValue = shrinkwrapParameters->getMaxElementNumber();
        std::cout << "Shrink wrap Max Element Number value for delete isolated volume is: " << maxElementValue << std::endl;
        std::cout << "Set Max Element Number is successful.\n";
    }

    // disable delete isolated volume setting, and check max element number setting.
    statusCode = shrinkwrapParameters->setDeleteIsolatedVolume(false);
    statusCode = shrinkwrapParameters->setMaxElementNumber(3);
    if (statusCode == PREPRO_BASE_NAMESPACE::PFStatus::ESetParameterNotPermitted)
    {
        std::cout << "You cannot set this parameter before other parameter(enable DeleteIsolatedVolume) are set correctly.\n";
    }
    
    maxElementValue = shrinkwrapParameters->getMaxElementNumber();
    std::cout << "Shrink wrap Max Element Number value for delete isolated volume is: " << maxElementValue << std::endl;
    std::cout << "**************************************************************\n";
}

void MeshDemo::createShrinkWrapMesh(const std::string& examplePath)
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }

    std::string filePath = examplePath + "mesh\\pipetee.stp";
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        return;
    }
    if (pfGeometry->importGeometry(filePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import geometry." << std::endl;
        return;
    }

    setMeshGlobalParameters(pfDocument);
    setMeshLocalSizeParameters(pfDocument);

    PREPRO_MESH_NAMESPACE::PFShrinkwrapParameters* shrinkwrapParameters = pfMesh->getShrinkwrapParameters();
    if (nullptr == shrinkwrapParameters)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus statusCode = shrinkwrapParameters->setGapCloseTolerance(0.02f);
    statusCode = shrinkwrapParameters->setGapCloseRefinement(1);
    statusCode = shrinkwrapParameters->setScaleDownFactor(0.9f);
    statusCode = shrinkwrapParameters->setLargestMethod();

    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createShrinkWrapMesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Wrap mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Wrap mesh creation failed." << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);

    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void MeshDemo::Remesh(const std::string& examplePath)
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }

    std::string filePath = examplePath + "mesh\\pipetee.stp";
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        return;
    }
    if (pfGeometry->importGeometry(filePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import geometry." << std::endl;
        return;
    }

    setMeshGlobalParameters(pfDocument);
    setMeshLocalSizeParameters(pfDocument);

    PREPRO_MESH_NAMESPACE::PFShrinkwrapParameters* shrinkwrapParameters = pfMesh->getShrinkwrapParameters();
    if (nullptr == shrinkwrapParameters)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus statusCode = shrinkwrapParameters->setGapCloseTolerance(0.02f);
    statusCode = shrinkwrapParameters->setGapCloseRefinement(1);
    statusCode = shrinkwrapParameters->setScaleDownFactor(0.9f);
    statusCode = shrinkwrapParameters->setLargestMethod();

    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createShrinkWrapMesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Wrap mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Wrap mesh creation failed." << std::endl;
        return;
    }

    // remesh, re-optimize the mesh, mainly for shrink wrap mesh.
    status = pfMesh->remesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Remesh is successful." << std::endl;
    }
    else
    {
        std::cout << "Remesh failed." << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);

    RenderDemo::show(pfData, VisualizationType::EMesh);
}

PREPRO_BASE_NAMESPACE::PFStatus MeshDemo::createVolumeMeshBySurfaceMesh(const std::string& path)
{
    int type = 1;
    std::cout << "Volume Mesh by Surface mesh, with different options:" << std::endl;
    std::cout << "#### 1. No inflation, no polyhedron. " << std::endl;
    std::cout << "#### 2. Has inflation, no polyhedron. " << std::endl;

    std::cin >> type;
    if (type > 2 || type <= 0)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr != meshBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            printf_s("No PERA SIM license");
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }
    }

    std::string meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\testInflation.ppcf";

    PFDocument* pfDocument = pfApplication.openDocument(meshFilePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "!!! Mesh is null." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createSurfaceMesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Surface mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Surface mesh creation failed." << std::endl;
        return status;
    }

    if (type == 1)
    {
        status = createVolumeMeshBySurfaceMeshNoInflationNoPolyhedron(pfMesh);
    }
    else if (type == 2)
    {
        PREPRO_MESH_NAMESPACE::PFInflationParameters* inflationParameters = pfMesh->getInflationParameters();
        if (nullptr == inflationParameters)
        {
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }

        // first must set inflation on before we use inflation setting.
        inflationParameters->setInflationOn("Volume1", "Share", true);

        PREPRO_BASE_NAMESPACE::PFStatus error = inflationParameters->setInitialHeight("Volume1", "Share", 0.00866462);
        if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::cout << "Failed to set initial height. Check parameter.\n";
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }

        error = inflationParameters->setLayerNumber("Volume1", "Share", 3);
        if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::cout << "Failed to set layer number. Check parameter.\n";
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }

        error = inflationParameters->setHeightRatio("Volume1", "Share", 1.25);
        if (error != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::cout << "Failed to set height ratio. Check parameter.\n";
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }
        status = createVolumeMeshBySurfaceMeshNeedInflationNoPolyhedron(pfMesh);
    }

    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Volume mesh by Surface mesh is failed.\n";
        return status;
    }
    else
    {
        std::cout << "Volume mesh by Surface mesh is successful.\n";
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);

    RenderDemo::show(pfData, VisualizationType::EMesh);

    return status;
}

PREPRO_BASE_NAMESPACE::PFStatus MeshDemo::createVolumeMeshBySurfaceMeshNoInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh)
{
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // Create volume mesh:
    // Without inflation and without generating polyhedron.
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createVolumeMeshBySurfaceMesh(false, false);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Volume mesh by Surface mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Volume mesh by Surface mesh creation failed." << std::endl;
    }
    return status;
}

PREPRO_BASE_NAMESPACE::PFStatus MeshDemo::createVolumeMeshBySurfaceMeshNeedInflationNoPolyhedron(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh)
{
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // Create volume mesh:
    // With inflation, without generating polyhedron.
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->createVolumeMeshBySurfaceMesh(true, false);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Volume mesh by Surface mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Volume mesh by Surface mesh creation failed." << std::endl;
    }
    return status;
}

void MeshDemo::computeLeakDetection(const std::string& path)
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
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return;
    }

    std::string filePath = path + "mesh\\pipetee.stp";
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        return;
    }
    if (pfGeometry->importGeometry(filePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import geometry." << std::endl;
        return;
    }

    setMeshGlobalParameters(pfDocument);
    setMeshLocalSizeParameters(pfDocument);

    PREPRO_MESH_NAMESPACE::PFShrinkwrapParameters* shrinkwrapParameters = pfMesh->getShrinkwrapParameters();
    if (nullptr == shrinkwrapParameters)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = shrinkwrapParameters->setGapCloseTolerance(0.0f);
    status = shrinkwrapParameters->setGapCloseRefinement(1);
    status = shrinkwrapParameters->setScaleDownFactor(1.0f);
    status = shrinkwrapParameters->setExternalMethod();
    status = shrinkwrapParameters->setKeepInternalShells(true);

    //before we do the leak detection, we generate one wrap mesh(surface mesh).
    status = pfMesh->createShrinkWrapMesh();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Wrap mesh creation is successful." << std::endl;
    }
    else
    {
        std::cout << "Wrap mesh creation failed." << std::endl;
        return;
    }

    // compute leak detection
    double source[3] = {-0.05, -0.05, -0.20};

    // target point should be inside the geometry bounding box. otherwise the compute leak detection will fail.
    double target[4][3] = {
        {0.0, 0.1, -0.10},
        {0.016, 0.12, -0.3},
        {0.014, 0.14, -0.23},
        {0.015, 0.132, -0.2735}
    };

    //use common path point type.
    PREPRO_MESH_NAMESPACE::PFLeakPathContainer totalLeakPath;
    status = pfMesh->computeLeakDetection(source, 4, target, totalLeakPath);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::ELeakPathFound)
    {
        for (int i = 0; i < totalLeakPath.getSize(); ++i)
        {
            PREPRO_MESH_NAMESPACE::PFLeakPathPointArray* pathArray = totalLeakPath.getArray(i);
            int arraySize = pathArray->getSize();
            for (int j = 0; j < arraySize; ++j)
            {
                double* point = pathArray->getPoint(j);
                std::cout << "point in leak path : " << point[0] << " " << point[1] << " " << point[2] << std::endl;
            }
            std::cout << "#########################" << std::endl;
        }
    }

    // show path
    RenderDemo::showLeakPath(totalLeakPath);
}

void MeshDemo::DensityDemo(const std::string& path)
{
    int type = 1;
    std::cout << "Select one density demo type: " << std::endl;
    std::cout << "#### 1. Open Density. " << std::endl;
    std::cout << "#### 2. Density test. " << std::endl;
    std::cout << "#### 3. Cylinder Density test." << std::endl;

    std::cin >> type;

    switch (type)
    {
    case 1:
        OpenDensity(path);
        break;
    case 2:
        DensityTest(path);
        break;
    case 3:
        CylinderDensityTest(path);
        break;
    default:
        break;
    }



}

void MeshDemo::OpenDensity(const std::string& path)
{
    std::string examplePath = path;

    // 1.new document,add geometry environment
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

    std::cout << "### Command working folder is: " << examplePath << std::endl;
    // 2. load document example file.
    std::string exampleFilePath = examplePath + "mesh\\density.ppcf";
    std::cout << "### Open ppcf path is: " << exampleFilePath << std::endl;

    PFDocument* pfDocument = pfApplication.openDocument(exampleFilePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return;
    }

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        std::cout << "!!! Geometry is null." << std::endl;
        return;
    }
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "!!! pfMesh is null." << std::endl;
        return;
    }

    // 3. get density settings.
    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);

    // 4. get mesh data from current document.
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);

    // 5. get density data from current document.
    std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> densityGroupList;    
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        PREPRO_BASE_NAMESPACE::PFGroup* group = new PFGroup(name, i, false);
        pfMesh->getDensityData(name, *group);
        densityGroupList.push_back(group);
    }

    // 6. render
    RenderDemo::show(pfData, VisualizationType::EMesh, densityGroupList);

    // 7. delete groupList.
    for (auto group : densityGroupList)
    {
        delete group;
    }
    densityGroupList.clear();
}

void MeshDemo::DensityTest(const std::string& path)
{
    std::string examplePath = path;
    PREPRO_BASE_NAMESPACE::PFStatus status = PREPRO_BASE_NAMESPACE::PFStatus::EError;

    // 1.new document,add geometry environment
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

    std::cout << "### Command working folder is: " << examplePath << std::endl;
    // 2. load document example file.
    std::string exampleFilePath = examplePath + "mesh\\density.ppcf";
    std::cout << "### Open ppcf path is: " << exampleFilePath << std::endl;

    PFDocument* pfDocument = pfApplication.openDocument(exampleFilePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return;
    }

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        std::cout << "!!! Geometry is null." << std::endl;
        return;
    }
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "!!! pfMesh is null." << std::endl;
        return;
    }

    //get density settings.
    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count: " << pfSettings.getCount() << std::endl;

    //3. rename density.
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        std::cout << "Index: " << i << ", Name: " << name << std::endl;
        std::string  newName = name;
        newName += "_rename";
        status = pfMesh->renameDensity(name, newName.c_str());
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::cout << "rename error: " << (int)status << ", Name: " << name << std::endl;
            return;
        }
    }

    // 4. delete density.
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count: " << pfSettings.getCount() << std::endl;
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        std::cout << "Index: " << i << ", Name: " << name << std::endl;
        status = pfMesh->deleteDensity(name);
        if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        {
            std::cout << "delete error: " << (int)status  <<", Name: " << name << std::endl;
            return;
        }
    }
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count(after delete): " << pfSettings.getCount() << std::endl;

    //5. add density.
    PREPRO_MESH_NAMESPACE::PFBoxDensitySetting box2("box2");
    double startPoint[3] = { 0.8,0.8, -0.2 };
    double endPoint[3] = { 1.2,1.2,0.2 };
    box2.setStartPoint(startPoint);
    box2.setEndPoint(endPoint);
    box2.setSize(0.01);
    box2.setRatio(1.1);
    status = pfMesh->createDensity(&box2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "create error: " << (int)status << ", Name: box2"<< std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFSphereDensitySetting sphere2("sphere2");
    double centerPoint[3] = { 0.0, 1.0, 0.0 };
    sphere2.setCenterPoint(centerPoint);
    sphere2.setRadius(0.3);
    sphere2.setSize(0.02);
    sphere2.setRatio(1.2);
    status = pfMesh->createDensity(&sphere2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "create error: " << (int)status << ", Name: sphere2" << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder2("cylinder2");
    double startPoint2[3] = { 1.0, 0.0, 1.0 };
    double endPoint2[3] = { 1.0, 1.0, 1.0 };
    cylinder2.setStartPoint(startPoint2);
    cylinder2.setEndPoint(endPoint2);
    cylinder2.setRadius(0.3);
    cylinder2.setSize(0.03);
    cylinder2.setRatio(1.3);
    status = pfMesh->createDensity(&cylinder2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "create error: " << (int)status << ", Name: cylinder2" << std::endl;
        return;
    }
    pfMesh->getDensitySettings(pfSettings);
    
    std::cout << "Density count(after add): " << pfSettings.getCount() << std::endl;

    //6. edit density.
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        auto type = setting->getType();
        bool result = false;
        switch (type)
        {
        case PFBase::PFDensityType::ENone:
            break;
        case PFBase::PFDensityType::EBox:
            result = DensityTestEditBox(pfMesh, dynamic_cast<const PREPRO_MESH_NAMESPACE::PFBoxDensitySetting*>(setting));
            break;
        case PFBase::PFDensityType::ESphere:
            result = DensityTestEditSphere(pfMesh, dynamic_cast<const PREPRO_MESH_NAMESPACE::PFSphereDensitySetting*>(setting));
            break;
        case PFBase::PFDensityType::ECylinder:
            result = DensityTestEditCylinder(pfMesh, dynamic_cast<const PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting*>(setting));
            break;
        default:
            break;
        }
        if (!result)
        {
            std::cout << "edit error:" << name << std::endl;
            return;
        }
    }
    std::cout << "---------------------  Density test success!"<< std::endl;
}

void MeshDemo::CylinderDensityTest(const std::string& path)
{
    std::string examplePath = path;

    // 1.new document,add geometry environment
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

    std::cout << "### Command working folder is: " << examplePath << std::endl;
    // 2. load document example file.
    std::string exampleFilePath = examplePath + "mesh\\density.ppcf";
    std::cout << "### Open ppcf path is: " << exampleFilePath << std::endl;

    PFDocument* pfDocument = pfApplication.openDocument(exampleFilePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return;
    }

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        std::cout << "!!! Geometry is null." << std::endl;
        return;
    }
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "!!! pfMesh is null." << std::endl;
        return;
    }
    //3. getDensitySettings
    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);
    //4.ClinderTest1
    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder("ClinderTest1");
    double startPoint[3] = { 0.0, 0.5, 0.0 };
    double endPoint[3] = { 0.0, 0.5, 1.0 };
    cylinder.setStartPoint(startPoint);
    cylinder.setEndPoint(endPoint);
    cylinder.setRadius(0.5);
    cylinder.setSize(0.03);
    cylinder.setRatio(1.3);
    pfMesh->createDensity(&cylinder);
    //5.ClinderTest2
    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder2("ClinderTest2");
    double startPoint2[3] = { 0.0, 0.853553, 0.146447 };
    double endPoint2[3] = { 0.0, 0.146447, 0.853553 };
    cylinder2.setStartPoint(startPoint2);
    cylinder2.setEndPoint(endPoint2);
    cylinder2.setRadius(0.5);
    cylinder2.setSize(0.03);
    cylinder2.setRatio(1.3);
    pfMesh->createDensity(&cylinder2);
    //6.ClinderTest3
    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder3("ClinderTest3");
    double startPoint3[3] = { -0.353553, 0.5, 0.146447 };
    double endPoint3[3] = { 0.353553, 0.5, 0.853553 };
    cylinder3.setStartPoint(startPoint3);
    cylinder3.setEndPoint(endPoint3);
    cylinder3.setRadius(0.5);
    cylinder3.setSize(0.03);
    cylinder3.setRatio(1.3);
    pfMesh->createDensity(&cylinder3);
    //7.ClinderTest4
    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder4("ClinderTest4");
    double startPoint4[3] = { 0, 0.5, 1.0 };
    double endPoint4[3] = { 0, 0.5, 0 };
    cylinder4.setStartPoint(startPoint4);
    cylinder4.setEndPoint(endPoint4);
    cylinder4.setRadius(0.5);
    cylinder4.setSize(0.03);
    cylinder4.setRatio(1.3);
    pfMesh->createDensity(&cylinder4);
    //8.ClinderTest5
    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting cylinder5("ClinderTest5");
    double startPoint5[3] = { 0.646447, 0.25, 0.25 };
    double endPoint5[3] = { 1.35355, 0.75, 0.75 };
    cylinder5.setStartPoint(startPoint5);
    cylinder5.setEndPoint(endPoint5);
    cylinder5.setRadius(0.5);
    cylinder5.setSize(0.03);
    cylinder5.setRatio(1.3);
    pfMesh->createDensity(&cylinder5);

}

bool MeshDemo::DensityTestEditBox(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFBoxDensitySetting* boxDensity)
{
    if (!boxDensity)
    {
        return false;
    }
    auto oldDensity = boxDensity;
    
    PREPRO_MESH_NAMESPACE::PFBoxDensitySetting newDensity = *oldDensity;
    double startPoint[3] = {};
    double endPoint[3] = {};
    double size = 0;
    double ratio = 0;
    oldDensity->getStartPoint(startPoint);
    oldDensity->getEndPoint(endPoint);
    size = oldDensity->getSize();
    ratio = oldDensity->getRatio();

    double delta = 0.001;
    double startPoint2[3] = {};
    double endPoint2[3] = {};
    double size2 = 0;
    double ratio2 = 0;
    startPoint2[0] = startPoint[0] + delta;
    startPoint2[1] = startPoint[1] + delta;
    startPoint2[2] = startPoint[2] + delta;
    endPoint2[0] = endPoint[0] + delta;
    endPoint2[1] = endPoint[1] + delta;
    endPoint2[2] = endPoint[2] + delta;
    size2 = size + delta;
    ratio2 = ratio + delta;
    newDensity.setStartPoint(startPoint2);
    newDensity.setEndPoint(endPoint2);
    newDensity.setSize(size2);
    newDensity.setRatio(ratio2);

    pfMesh->editDensity(&newDensity);
    double startPoint3[3] = {};
    double endPoint3[3] = {};
    double size3 = 0;
    double ratio3 = 0;

    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count: " << pfSettings.getCount() << std::endl;
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        if (strcmp(name, newDensity.getName()) == 0)
        {
            const PREPRO_MESH_NAMESPACE::PFBoxDensitySetting* boxEdit = dynamic_cast<const PREPRO_MESH_NAMESPACE::PFBoxDensitySetting*>(setting);
            if (!boxEdit)
            {
                return false;
            }
            boxEdit->getStartPoint(startPoint3);
            boxEdit->getEndPoint(endPoint3);
            size3 = boxEdit->getSize();
            ratio3 = boxEdit->getRatio();
            
            if (!approximatelyEqual(startPoint3[0], startPoint2[0])
                || !approximatelyEqual(startPoint3[1], startPoint2[1])
                || !approximatelyEqual(startPoint3[2], startPoint2[2])
                || !approximatelyEqual(endPoint3[0], endPoint2[0])
                || !approximatelyEqual(endPoint3[1], endPoint2[1])
                || !approximatelyEqual(endPoint3[2], endPoint2[2])
                || !approximatelyEqual(size3, size2)
                || !approximatelyEqual(ratio3, ratio2))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}

bool MeshDemo::DensityTestEditSphere(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFSphereDensitySetting* sphereDensity)
{
    if (!sphereDensity)
    {
        return false;
    }
    auto oldDensity = sphereDensity;

    PREPRO_MESH_NAMESPACE::PFSphereDensitySetting newDensity = *oldDensity;
    double centerPoint[3] = {};
    double radius = 0;
    double size = 0;
    double ratio = 0;
    oldDensity->getCenterPoint(centerPoint);
    radius = oldDensity->getRadius();
    size = oldDensity->getSize();
    ratio = oldDensity->getRatio();

    double delta = 0.001;
    double centerPoint2[3] = {};
    double radius2 = 0;
    double size2 = 0;
    double ratio2 = 0;
    centerPoint2[0] = centerPoint[0] + delta;
    centerPoint2[1] = centerPoint[1] + delta;
    centerPoint2[2] = centerPoint[2] + delta;
    radius2 = radius + delta;
    size2 = size + delta;
    ratio2 = ratio + delta;
    newDensity.setCenterPoint(centerPoint2);
    newDensity.setRadius(radius2);
    newDensity.setSize(size2);
    newDensity.setRatio(ratio2);

    pfMesh->editDensity(&newDensity);
    double centerPoint3[3] = {};
    double radius3 = 0;
    double size3 = 0;
    double ratio3 = 0;

    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count: " << pfSettings.getCount() << std::endl;
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        if (strcmp(name, newDensity.getName()) == 0)
        {
            const PREPRO_MESH_NAMESPACE::PFSphereDensitySetting* sphereEdit = dynamic_cast<const PREPRO_MESH_NAMESPACE::PFSphereDensitySetting*>(setting);
            if (!sphereEdit)
            {
                return false;
            }
            sphereEdit->getCenterPoint(centerPoint3);
            radius3 = sphereEdit->getRadius();
            size3 = sphereEdit->getSize();
            ratio3 = sphereEdit->getRatio();

            if (!approximatelyEqual(centerPoint3[0], centerPoint2[0])
                || !approximatelyEqual(centerPoint3[1], centerPoint2[1])
                || !approximatelyEqual(centerPoint3[2], centerPoint2[2])
                || !approximatelyEqual(radius3, radius2)
                || !approximatelyEqual(size3, size2)
                || !approximatelyEqual(ratio3, ratio2))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}

bool MeshDemo::DensityTestEditCylinder(PREPRO_MESH_NAMESPACE::PFMesh* pfMesh, const PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting* cylinderDensity)
{
    if (!cylinderDensity)
    {
        return false;
    }
    auto oldDensity = cylinderDensity;

    PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting newDensity = *oldDensity;
    double startPoint[3] = {};
    double endPoint[3] = {};
    double radius = 0;
    double size = 0;
    double ratio = 0;
    oldDensity->getStartPoint(startPoint);
    oldDensity->getEndPoint(endPoint);
    radius = oldDensity->getRadius();
    size = oldDensity->getSize();
    ratio = oldDensity->getRatio();

    double delta = 0.001;
    double startPoint2[3] = {};
    double endPoint2[3] = {};
    double radius2 = 0;
    double size2 = 0;
    double ratio2 = 0;
    startPoint2[0] = startPoint[0] + delta;
    startPoint2[1] = startPoint[1] + delta;
    startPoint2[2] = startPoint[2] + delta;
    endPoint2[0] = endPoint[0] + delta;
    endPoint2[1] = endPoint[1] + delta;
    endPoint2[2] = endPoint[2] + delta;
    radius2 = radius + delta;
    size2 = size + delta;
    ratio2 = ratio + delta;
    newDensity.setStartPoint(startPoint2);
    newDensity.setEndPoint(endPoint2);
    newDensity.setRadius(radius2);
    newDensity.setSize(size2);
    newDensity.setRatio(ratio2);

    pfMesh->editDensity(&newDensity);
    double startPoint3[3] = {};
    double endPoint3[3] = {};
    double radius3 = 0;
    double size3 = 0;
    double ratio3 = 0;

    PREPRO_MESH_NAMESPACE::PFDensitySettings pfSettings;
    pfMesh->getDensitySettings(pfSettings);
    std::cout << "Density count: " << pfSettings.getCount() << std::endl;
    for (unsigned int i = 0; i < pfSettings.getCount(); ++i)
    {
        auto setting = pfSettings.getSetting(i);
        auto name = setting->getName();
        if (strcmp(name, newDensity.getName()) == 0)
        {
            const PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting* cylinderEdit = dynamic_cast<const PREPRO_MESH_NAMESPACE::PFCylinderDensitySetting*>(setting);
            if (!cylinderEdit)
            {
                return false;
            }
            cylinderEdit->getStartPoint(startPoint3);
            cylinderEdit->getEndPoint(endPoint3);
            radius3 = cylinderEdit->getRadius();
            size3 = cylinderEdit->getSize();
            ratio3 = cylinderEdit->getRatio();

            if (!approximatelyEqual(startPoint3[0], startPoint2[0])
                || !approximatelyEqual(startPoint3[1], startPoint2[1])
                || !approximatelyEqual(startPoint3[2], startPoint2[2])
                || !approximatelyEqual(endPoint3[0], endPoint2[0])
                || !approximatelyEqual(endPoint3[1], endPoint2[1])
                || !approximatelyEqual(endPoint3[2], endPoint2[2])
                || !approximatelyEqual(size3, size2)
                || !approximatelyEqual(ratio3, ratio2))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}

bool MeshDemo::approximatelyEqual(double a, double b)
{
#define TOLERANCE 1e-4
    return fabs(a - b) < TOLERANCE;
}

void MeshDemo::convertGeometryToMesh(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (nullptr == geomertyBuilder)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = pfApplication.addEnvironment(geomertyBuilder);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to add geometry environment." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr == meshBuilder)
    {
        return;
    }
    status = pfApplication.addEnvironment(meshBuilder);
    if (status!=PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to add mesh environment." << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        std::cout << "Failed to new document." << std::endl;
        return;
    }
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        return;
    }
    //import geometry
    std::string filePath = examplePath + "mesh\\pipetee.stp";
    status = pfGeometry->importGeometry(filePath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import geometry." << std::endl;
        return;
    }
    //create cylinder
    float startPoint[3] = { 0.0,0.0,0.0 };
    float endPoint[3] = { 0.2f,0.0,0.0 };
    status = pfGeometry->createCylinder("cylinder1", startPoint, endPoint, 0.1f);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create cylinder." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "PFMesh is null." << std::endl;
        return;
    }
    //convert geometry to mesh
    PREPRO_BASE_NAMESPACE::PFData meshData;
    status = pfMesh->convertGeometryToMesh();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to convert geometry to mesh." << std::endl;
        return;
    }
    pfMesh->getAllData(meshData);
    RenderDemo::show(meshData, VisualizationType::EMesh);
}


void MeshDemo::cutPlaneDemo(const std::string& path)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (nullptr == geomertyBuilder)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = pfApplication.addEnvironment(geomertyBuilder);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to add geometry environment." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr == meshBuilder)
    {
        return;
    }
    status = pfApplication.addEnvironment(meshBuilder);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to add mesh environment." << std::endl;
        return;
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        std::cout << "Failed to new document." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "PFMesh is null." << std::endl;
        return;
    }
    std::string meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\demo.ppcf";
    status = pfMesh->importMesh(meshFilePath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
        return;
    }
    double center[3] = { 25.4,0.,0. };
    double normal[3] = { 1.,0.,0. };
    PREPRO_BASE_NAMESPACE::PFData meshData;
    status = pfMesh->cutPlane(center, normal, meshData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Cut plane failed:" << meshFilePath << std::endl;
        return;
    }
    RenderDemo::show(meshData, VisualizationType::EMesh);

    meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\HECHT-POLY-cgns-HDF5.cgns";
    status = pfMesh->importMesh(meshFilePath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
        return;
    }
    double hechtCenter[3] = { -0.1,-0.2,0.3 };
    double hechtNormal[3] = { 1.,0.,1. };
    status = pfMesh->cutPlane(hechtCenter, hechtNormal, meshData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Cut plane failed:" << meshFilePath << std::endl;
        return;
    }
    RenderDemo::show(meshData, VisualizationType::EMesh);

    meshFilePath = CommonMethods::getCurrentProgramPath(path) + "\\mesh\\Pipetee.ppcf";
    status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EReplace);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Import mesh file failed:" << meshFilePath << std::endl;
        return;
    }
    double pipeteeCenter[3] = { 0.01,0.2,-0.2 };
    double pipeteeNormal[3] = { 3.,-1.,-2. };
    status = pfMesh->cutPlane(pipeteeCenter, pipeteeNormal, meshData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Cut plane failed:" << meshFilePath << std::endl;
        return;
    }
    RenderDemo::show(meshData, VisualizationType::EMesh);
}