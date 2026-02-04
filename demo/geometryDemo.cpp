// Revision: $Id: geometryDemo.cpp,v 1.0 2025/02/24 13:55:04 zhiqiang.chen Exp $
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

#include "GeometryDemo.h"
#include "renderDemo.h"

#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometryData.h>
#include <mesh/pfMeshBuilder.h>
#include <mesh/pfMesh.h>

#include <iostream>
#include <unordered_map>
#include <string>

#include "commonMethods.h"

void GeometryDemo::geometryDemo(const std::string& examplePath)
{
    //1
    //get running function
    std::pair<std::string, func> result = getFunctionAndFileName();
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;

    //2
    //Get a pointer to the API export class object
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = getPFGeometryPointer(pfApplication, examplePath, result.first);

    //3
    //running
    result.second(pfGeometry);

    //4
    //show
    if (pfGeometry)
    {
        PREPRO_BASE_NAMESPACE::PFData geometryData;
        pfGeometry->getAllData(geometryData);
        RenderDemo::show(geometryData, VisualizationType::EGeomerty);
    }
}

#define FUNCTION_NAME_TO_STRING(func) #func
std::pair<std::string, func> GeometryDemo::getFunctionAndFileName()
{
    std::unordered_map<int, std::pair<std::string, func>> typeSet = { {1,std::make_pair(FUNCTION_NAME_TO_STRING(quickRepair),quickRepair)},
                                                                      {2,std::make_pair(FUNCTION_NAME_TO_STRING(findVolumes),findVolumes)},
                                                                      {3,std::make_pair(FUNCTION_NAME_TO_STRING(stitch),stitch)},
                                                                      {4,std::make_pair(FUNCTION_NAME_TO_STRING(defeature),defeature)},
                                                                      {5,std::make_pair(FUNCTION_NAME_TO_STRING(intersect),intersect)},
                                                                      {6,std::make_pair(FUNCTION_NAME_TO_STRING(stitchHole),stitchHole)},
                                                                      {7,std::make_pair(FUNCTION_NAME_TO_STRING(fillHole),fillHole)},
                                                                      {8,std::make_pair(FUNCTION_NAME_TO_STRING(fillAllHoles),fillAllHoles)},
                                                                      {9,std::make_pair(FUNCTION_NAME_TO_STRING(fillFacesHoles),fillFacesHoles)},
                                                                      {10,std::make_pair(FUNCTION_NAME_TO_STRING(addEntitiesToGroup),addEntitiesToGroup)},
                                                                      {11,std::make_pair(FUNCTION_NAME_TO_STRING(createEdge),createEdge)},
                                                                      {12,std::make_pair(FUNCTION_NAME_TO_STRING(splitEdge),splitEdge)},
                                                                      {13,std::make_pair(FUNCTION_NAME_TO_STRING(mergeEdges),mergeEdges)},
                                                                      {14,std::make_pair(FUNCTION_NAME_TO_STRING(createFaceByEdges),createFaceByEdges)},
                                                                      {15,std::make_pair(FUNCTION_NAME_TO_STRING(splitFaceByEdge),splitFaceByEdge)},
                                                                      {17,std::make_pair(FUNCTION_NAME_TO_STRING(createFaceByPoints),createFaceByPoints)},
                                                                      {20,std::make_pair(FUNCTION_NAME_TO_STRING(project),project)},
                                                                      {16,std::make_pair(FUNCTION_NAME_TO_STRING(mergeFaces),mergeFaces)},
                                                                      {21,std::make_pair(FUNCTION_NAME_TO_STRING(createCube),createCube)},
                                                                      {22,std::make_pair(FUNCTION_NAME_TO_STRING(createCylinder),createCylinder)},
                                                                      {24,std::make_pair(FUNCTION_NAME_TO_STRING(createHemisphere),createHemisphere)},
                                                                      {23,std::make_pair(FUNCTION_NAME_TO_STRING(createSphere),createSphere)},
                                                                      {25,std::make_pair(FUNCTION_NAME_TO_STRING(deleteEntities),deleteEntities)},
                                                                      {18,std::make_pair(FUNCTION_NAME_TO_STRING(imprint),imprint)},
                                                                      {19,std::make_pair(FUNCTION_NAME_TO_STRING(extrude),extrude)},
                                                                      {27,std::make_pair(FUNCTION_NAME_TO_STRING(rotate),rotate)},
                                                                      {28,std::make_pair(FUNCTION_NAME_TO_STRING(mirror),mirror)},
                                                                      {29,std::make_pair(FUNCTION_NAME_TO_STRING(scale),scale)},
                                                                      {26,std::make_pair(FUNCTION_NAME_TO_STRING(translate),translate)},
                                                                      {34,std::make_pair(FUNCTION_NAME_TO_STRING(createVolume),createVolume)},
                                                                      {33,std::make_pair(FUNCTION_NAME_TO_STRING(detachVolumes),detachVolumes)},
                                                                      {30,std::make_pair(FUNCTION_NAME_TO_STRING(deleteFaceGroups),deleteFaceGroups)},
                                                                      {32,std::make_pair(FUNCTION_NAME_TO_STRING(rename),rename)},
                                                                      {31,std::make_pair(FUNCTION_NAME_TO_STRING(deleteVolumes),deleteVolumes)}, 
                                                                      {35,std::make_pair(FUNCTION_NAME_TO_STRING(convertMeshToGeometry),convertMeshToGeometry) }
};


    std::cout << "All runnable interface types and numbers are as follows: " << std::endl;
    for (auto iter = typeSet.begin(); iter != typeSet.end(); iter++)
    {
        std::cout << iter->second.first << "  " << std::to_string(iter->first) << std::endl;
    }

    std::cout << "Please enter the number of the interface to be run :";
    int number = 0;
    std::cin >> number;

    auto iter = typeSet.find(number);
    if (iter == typeSet.end())
    {
        std::cout << "Non-existent running type !" << std::endl;
        return std::pair<std::string, func>();
    }

    return iter->second;
}

PREPRO_GEOMETRY_NAMESPACE::PFGeometry* GeometryDemo::getPFGeometryPointer(PREPRO_BASE_NAMESPACE::PFApplication& pfApplication, const std::string& examplePath, const std::string& fileName)
{
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            printf_s("No PERA SIM license");
            return nullptr;
        }
    }
    else
    {
        std::cout << "No geometry information, demo failed !" << std::endl;
        return nullptr;
    }
    if (fileName == FUNCTION_NAME_TO_STRING(convertMeshToGeometry))
    {
        return importMeshBeforeConverttingMeshToGeometry(pfApplication, examplePath);
    }

    std::string geometryFilePath = examplePath +"geometry\\" + fileName;
    if (fileName == FUNCTION_NAME_TO_STRING(quickRepair) || fileName == FUNCTION_NAME_TO_STRING(findVolumes) || fileName == FUNCTION_NAME_TO_STRING(mergeEdges)
        || fileName == FUNCTION_NAME_TO_STRING(splitFaceByEdge) || fileName == FUNCTION_NAME_TO_STRING(mergeFaces) || fileName == FUNCTION_NAME_TO_STRING(imprint)
        || fileName == FUNCTION_NAME_TO_STRING(extrude) || fileName == FUNCTION_NAME_TO_STRING(createVolume) || fileName == FUNCTION_NAME_TO_STRING(detachVolumes)
        || fileName == FUNCTION_NAME_TO_STRING(deleteFaceGroups) || fileName == FUNCTION_NAME_TO_STRING(rename) || fileName == FUNCTION_NAME_TO_STRING(deleteVolumes))
    {
        geometryFilePath += ".ppcf";
    }
    else if (fileName== FUNCTION_NAME_TO_STRING(defeature))
    {
        geometryFilePath += ".igs";
    }
    else if (fileName == FUNCTION_NAME_TO_STRING(stitchHole) || fileName == FUNCTION_NAME_TO_STRING(fillHole) || fileName == FUNCTION_NAME_TO_STRING(fillAllHoles) || fileName == FUNCTION_NAME_TO_STRING(addEntitiesToGroup)
        || fileName == FUNCTION_NAME_TO_STRING(createEdge) || fileName == FUNCTION_NAME_TO_STRING(splitEdge) || fileName == FUNCTION_NAME_TO_STRING(createFaceByEdges) || fileName == FUNCTION_NAME_TO_STRING(createFaceByPoints)
        || fileName == FUNCTION_NAME_TO_STRING(project) || fileName == FUNCTION_NAME_TO_STRING(deleteEntities) || fileName == FUNCTION_NAME_TO_STRING(scale) || fileName == FUNCTION_NAME_TO_STRING(mirror)
        || fileName == FUNCTION_NAME_TO_STRING(rotate) || fileName == FUNCTION_NAME_TO_STRING(translate))
    {
        geometryFilePath += ".stl";
    }
    else if (fileName == FUNCTION_NAME_TO_STRING(stitch) || fileName == FUNCTION_NAME_TO_STRING(intersect))
    {
        geometryFilePath += ".stp";
    }
    else if (fileName == FUNCTION_NAME_TO_STRING(fillFacesHoles))
    {
        geometryFilePath += ".x_t";
    }
    else if (fileName == FUNCTION_NAME_TO_STRING(createCube) || fileName == FUNCTION_NAME_TO_STRING(createCylinder) || fileName == FUNCTION_NAME_TO_STRING(createHemisphere)
        || fileName == FUNCTION_NAME_TO_STRING(createSphere))
    {
        geometryFilePath = "";
    }
    else
    {
        return nullptr;
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();

    //get PFGeometry from PFDocument
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (!pfGeometry)
    {
        std::cout << "No geometry information, demo failed !" << std::endl;
        return nullptr;
    }

    if (!geometryFilePath.empty() && pfGeometry->importGeometry(geometryFilePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Importing geometry file failed !" << std::endl;
        return nullptr;
    }

    return pfGeometry;
}

void GeometryDemo::quickRepair(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Initialized Data
    PREPRO_GEOMETRY_NAMESPACE::QuickRepairParameters parameters;
    parameters.stitchOption = PREPRO_GEOMETRY_NAMESPACE::StitchOptions::EStitchAndIntersection;
    parameters.stitchTolerance = 1e-5;
    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->quickRepair(parameters);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Quick repair successfully !" << std::endl;
    }
    else
    {
        std::cout << "Quick repair failed !" << std::endl;
    }
}

void GeometryDemo::findVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->findVolumes();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Find volumes successfully !" << std::endl;
    }
    else
    {
        std::cout << "Find volumes failed !" << std::endl;
    }

    PREPRO_BASE_NAMESPACE::PFData geometryData;
    geometry->getAllData(geometryData);
    unsigned int volumeNum = geometryData.getVolumeSize();
    std::cout << "The volume quantity found is:" + std::to_string(volumeNum) << std::endl;
}

void GeometryDemo::stitch(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int ids[2] = { 17,18 };

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->stitchTwoFaces(ids, 0.001);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Stitching two faces successfully !" << std::endl;
    }
    else
    {
        std::cout << "Stitching two faces failed !" << std::endl;
    }
}

void GeometryDemo::defeature(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->defeature(1e-6);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Remove features successfully !" << std::endl;
    }
    else
    {
        std::cout << "Remove features failed !" << std::endl;
    }
}

void GeometryDemo::stitchHole(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //call interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->stitchHole(26);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Stitching hole successfully !" << std::endl;
    }
    else
    {
        std::cout << "Stitching hole failed !" << std::endl;
    }
}

void GeometryDemo::intersect(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int ids[2] = { 17,18 };
    unsigned int num = 0;

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->intersectTwoFaces(ids, 0.001);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Two faces intersect successfully !" << std::endl;
    }
    else
    {
        std::cout << "Two faces intersect failed !" << std::endl;
    }
}

void GeometryDemo::fillHole(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //call interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->fillHole(19);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Filling hole successfully !" << std::endl;
    }
    else
    {
        std::cout << "Filling hole failed !" << std::endl;
    }
}

void GeometryDemo::fillAllHoles(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->fillAllHoles();
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Filling holes successfully !" << std::endl;
    }
    else
    {
        std::cout << "Filling holes failed !" << std::endl;
    }
}

void GeometryDemo::fillFacesHoles(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int faceIds[1];
    faceIds[0] = 27;
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->fillFacesHoles(1, faceIds);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Filling holes successfully !" << std::endl;
    }
    else
    {
        std::cout << "Filling holes failed !" << std::endl;
    }
}

void GeometryDemo::addEntitiesToGroup(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //group faces
    unsigned int faceIds[1];
    faceIds[0] = 43;
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->addEntitiesToGroup(1, faceIds, "group2");
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Group faces successfully !" << std::endl;
    }
    else
    {
        std::cout << "Group faces failed !" << std::endl;
    }
}

void GeometryDemo::createEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //call interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createEdge(15, 9);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create edge successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create edge failed !" << std::endl;
    }
}

void GeometryDemo::splitEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->splitEdge(34, 0.5);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Split edge successful !" << std::endl;
    }
    else
    {
        std::cout << "Split edge failed !" << std::endl;
    }
}

void GeometryDemo::mergeEdges(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int edgeIds[2] = { 22,52 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->mergeEdges(2, edgeIds);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Merge edges successful !" << std::endl;
    }
    else
    {
        std::cout << "Merge edges failed !" << std::endl;
    }
}

void GeometryDemo::createFaceByEdges(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int edgeIds[6] = { 33,29,40,38,32,39 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createFaceByEdges(6, edgeIds);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create face successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create face failed !" << std::endl;
    }
}

void GeometryDemo::splitFaceByEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->splitFaceByEdge(55, 44);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Splitting face successfully !" << std::endl;
    }
    else
    {
        std::cout << "Splitting face failed !" << std::endl;
    }
}

void GeometryDemo::createFaceByPoints(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    double coordinate[12] = { 0.0, 0.049897190183401108, 0.050000000745058060, 0.0, 0.050000000745058060, 0.00030813505873084068,
                            0.0, 0.0, 0.00034793704980984330, 0.0, 0.0, 0.050000000745058060 };

    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createFaceByPoints(4, coordinate);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create face successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create face failed !" << std::endl;
    }
}

void GeometryDemo::project(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->project(16, 42);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Project successful !" << std::endl;
    }
    else
    {
        std::cout << "Project failed !" << std::endl;
    }
}

void GeometryDemo::mergeFaces(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int faceIds[2] = { 44,56 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->mergeFaces(2, faceIds);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Merge faces successfully !" << std::endl;
    }
    else
    {
        std::cout << "Merge faces failed !" << std::endl;
    }
}

void GeometryDemo::extrude(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int ids = 25;
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->extrude(1, &ids, PREPRO_GEOMETRY_NAMESPACE::EExtrudeType::ENormal, 0.02);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Extrude successful !" << std::endl;
    }
    else
    {
        std::cout << "Extrude failed !" << std::endl;
    }
}

void GeometryDemo::createCube(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }
    float startPoint[3] = { 0.0,0.0,0.0 };
    float endPoint[3] = { 0.2,0.2,0.2 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createCube("Cubic_1", startPoint, endPoint);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create cube successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create cube failed !" << std::endl;
    }
}

void GeometryDemo::createCylinder(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    float startPoint[3] = { 0.0,0.0,0.0 };
    float endPoint[3] = { 1.0,0.0,0.0 };

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createCylinder("Cylinder_1", startPoint, endPoint, 1.0);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create cylinder successful !" << std::endl;
    }
    else
    {
        std::cout << "Create cylinder failed !" << std::endl;
    }
}

void GeometryDemo::createHemisphere(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    float centerCoordinates[3] = { 0.0 };
    float direction[3] = { 1.0,0.0,0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createHemisphere("Hemisphere_1", centerCoordinates, direction, 1.0);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create hemisphere successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create hemisphere failed !" << std::endl;
    }
}

void GeometryDemo::createSphere(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    float center[3] = { 0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createSphere("Sphere_1", center, 1.0);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create sphere successfully !" << std::endl;
    }
    else
    {
        std::cout << "Create sphere failed !" << std::endl;
    }
}

void GeometryDemo::deleteEntities(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    unsigned int ids[1] = { 44 };

    //Call Interface
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->deleteEntities(1, ids, PREPRO_GEOMETRY_NAMESPACE::DeleteOptions::EKeepAffiliatedEntities);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete entities successful !" << std::endl;
    }
    else
    {
        std::cout << "Delete entities failed !" << std::endl;
    }
}

void GeometryDemo::imprint(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int faceIds[2] = { 25,33 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->imprint(2, faceIds);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Imprint successful !" << std::endl;
    }
    else
    {
        std::cout << "Imprint failed !" << std::endl;
    }
}

void GeometryDemo::scale(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int ids[1] = { 44 };
    double origin[3] = { 0.0,0.0,0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->scale(1, ids, origin, 1.5, PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Scale successful !" << std::endl;
    }
    else
    {
        std::cout << "Scale failed !" << std::endl;
    }
}

void GeometryDemo::mirror(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int ids[3] = { 15, 29, 44 };
    double origin[3] = { 0.0,0.0,0.0 };
    double direction[3] = { 1.0,0.0,0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->mirror(3, ids, origin, direction, PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Mirror successful !" << std::endl;
    }
    else
    {
        std::cout << "Mirror failed !" << std::endl;
    }
}

void GeometryDemo::rotate(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int ids[3] = { 15, 29, 44 };
    double origin[3] = { 0.0,0.0,0.0 };
    double direction[3] = { 1.0,0.0,0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->rotate(3, ids, origin, direction, 90, PREPRO_GEOMETRY_NAMESPACE::TransformOptions::EOnlyTransform);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Rotate successful !" << std::endl;
    }
    else
    {
        std::cout << "Rotate failed !" << std::endl;
    }
}

void GeometryDemo::translate(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    unsigned int ids[3] = { 15, 29, 44 };
    double offset[3] = { 0.5,0.0,0.0 };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->translate(3, ids, offset, PREPRO_GEOMETRY_NAMESPACE::TransformOptions::ECopyAndTransform);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Translate successful !" << std::endl;
    }
    else
    {
        std::cout << "Translate failed !" << std::endl;
    }
}

void GeometryDemo::createVolume(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    const char** groupNames = new const char* [1];
    groupNames[0] = { "b2" };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->createVolume(1, groupNames);
    delete[]groupNames;
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create volume successful !" << std::endl;
    }
    else
    {
        std::cout << "Create volume failed !" << std::endl;
    }
}

void GeometryDemo::detachVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    const char** volumeNames = new const char* [1];
    volumeNames[0] = { "Volume1" };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->detachVolumes(1, volumeNames);
    delete[]volumeNames;
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Detach volumes successful !" << std::endl;
    }
    else
    {
        std::cout << "Detach volumes failed !" << std::endl;
    }
}

void GeometryDemo::deleteFaceGroups(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    const char** groupNames = new const char* [1];
    groupNames[0] = { "j_shape_1" };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->deleteFaceGroups(1, groupNames);
    delete[]groupNames;
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete face groups successful !" << std::endl;
    }
    else
    {
        std::cout << "Delete face groups failed !" << std::endl;
    }
}

void GeometryDemo::rename(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->renameFaceGroup("b2_unused", "b2_unused_test");
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Rename face group successful !" << std::endl;
    }
    else
    {
        std::cout << "Rename face group failed !" << std::endl;
    }

    status = geometry->renameVolume("Volume1", "Volume1_test");
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Rename volume successful !" << std::endl;
    }
    else
    {
        std::cout << "Rename volume failed !" << std::endl;
    }
}

void GeometryDemo::deleteVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }

    //Call Interface
    const char** volumeNames = new const char* [1];
    volumeNames[0] = { "Volume1" };
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->deleteVolumes(1, volumeNames);
    delete[]volumeNames;
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete volumes successful !" << std::endl;
    }
    else
    {
        std::cout << "Delete volumes failed !" << std::endl;
    }
}

void GeometryDemo::convertMeshToGeometry(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry)
{
    if (!geometry)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->convertMeshToGeometry();
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to convert mesh to geometry." << std::endl;
    }
}

PREPRO_GEOMETRY_NAMESPACE::PFGeometry* GeometryDemo::importMeshBeforeConverttingMeshToGeometry(PREPRO_BASE_NAMESPACE::PFApplication& pfApplication, const std::string& examplePath)
{
    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr == meshBuilder)
    {
        return nullptr;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = pfApplication.addEnvironment(meshBuilder);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to add mesh environment." << std::endl;
        return nullptr;
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        std::cout << "PFMesh is null." << std::endl;
        return nullptr;
    }
    std::string filePath = examplePath + "geometry\\" + FUNCTION_NAME_TO_STRING(convertMeshToGeometry) + ".msh";
    if (!filePath.empty() && pfMesh->importMesh(filePath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Importing mesh file failed !" << std::endl;
        return nullptr;
    }

    //get PFGeometry from PFDocument
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (!pfGeometry)
    {
        std::cout << "No geometry information, demo failed !" << std::endl;
        return nullptr;
    }
    return pfGeometry;
}
