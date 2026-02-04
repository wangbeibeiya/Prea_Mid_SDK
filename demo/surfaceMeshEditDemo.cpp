// Revision: $Id: surfaceMeshEditDemo.cpp,v 1.0 2025/04/02 13:15:20 cuianzhu Exp $
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

#include <iostream>
#include "surfaceMeshEditDemo.h"
#include "base/pfApplication.h"
#include "base/pfDocument.h"
#include "base/common/pfEnumeration.h"
#include "base/pfGroupData.h"
#include "mesh/pfmesh.h"
#include "mesh/pfMeshBuilder.h"
#include "geometry/pfGeometryBuilder.h"
#include "geometry/pfGeometry.h"
#include "renderDemo.h"
#include "mesh/pfSurfaceSmoothParameter.h"

#define OPEN_DOCUMENT(path) std::string filePath = path;\
PREPRO_BASE_NAMESPACE::PFApplication pfApplication;\
PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();\
if (nullptr != geomertyBuilder)\
{\
    if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(geomertyBuilder))\
    {\
        printf_s("No PERA SIM license");\
        return;\
    }\
}\
PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();\
if (nullptr != meshBuilder)\
{\
    if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))\
    {\
        std::cout << "No PERA SIM license" << std::endl;\
        return;\
    }\
}\
std::cout << "### Open ppcf path is: " << filePath << std::endl;\
PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.openDocument(filePath.c_str());\
if (nullptr == pfDocument)\
{\
    std::cout << "!!! Document open failed." << std::endl;\
    return;\
}\
PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());\
if (nullptr == pfGeometry)\
{\
    std::cout << "!!! Geometry is null." << std::endl;\
    return;\
}\
PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());\
if (nullptr == pfMesh)\
{\
    std::cout << "!!! pfMesh is null." << std::endl;\
    return;\
}

void SurfaceMeshEditDemo::surfaceMeshEditDemo(const std::string& path)
{
    int type = 0;
    std::cout << "Select one demo for surface mesh edit: " << std::endl;
    std::cout << "#### 1. Create Triangle. " << std::endl;
    std::cout << "#### 2. Project. " << std::endl;
    std::cout << "#### 3. MoveToGroup. " << std::endl;
    std::cout << "#### 4. Intersect." << std::endl;
    std::cout << "#### 5. FillHoles." << std::endl;
    std::cout << "#### 6. Split." << std::endl;
    std::cout << "#### 7. SmoothCells." << std::endl;
    std::cout << "#### 8. SwapEdge." << std::endl;
    std::cout << "#### 9. RemeshCells." << std::endl;
    std::cout << "#### 10. CollapseEdges." << std::endl;
    std::cout << "#### 11. DeleteCells." << std::endl;

    std::cin >> type;
    switch (type)
    {
    case 1:
        createDemo(path);
        break;
    case 2:
        projectNodesDemo(path);
        break;
    case 3:
        moveToGroupDemo(path);
        break;
    case 4:
        intersectDemo(path);
        break;
    case 5:
        fillHolesDemo(path);
        break;
    case 6:
        splitDemo(path);
        break;
    case 7:
        smoothCellsDemo(path);
        break;
    case 8:
        swapEdgeDemo(path);
        break;
    case 9:
        remeshCellsDemo(path);
        break;
    case 10:
        collapseEdgesDemo(path);
        break;
    case 11:
        deleteCellsDemo(path);
        break;
    default:
        break;
    }
}

void SurfaceMeshEditDemo::createDemo(const std::string& path)
{
    int type = 0;
    std::cout << "Select one demo for create: " << std::endl;
    std::cout << "#### 1. By 3 points. " << std::endl;
    std::cout << "#### 2. By 3 edges. " << std::endl;

    std::cin >> type;
    switch (type)
    {
    case 1:
        createSurfaceMeshBy3Points(path);
        break;
    case 2:
        createSurfaceMeshBy3Edges(path);
        break;
    default:
        break;
    }
}

void SurfaceMeshEditDemo::createSurfaceMeshBy3Points(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
    //create surface mesh, by 3 points.
    int idOfCells[3] = { 0, 8, 9 };   //The id of the cell to which the point belongs.
    //x,y,z for 3 points.
    double points[3][3] = {
        {0.8, 1, 1},            //point1
        {1, 0.8, 1 },           //point2
        {0.8, 0.8, 1}           //point3
    };
    pfMesh->createTriangle(idOfCells, 3, points);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::createSurfaceMeshBy3Edges(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
    //create surface mesh, by 3 edges.
    int idOfCells[3] = { 0, 8, 4 };//The id of the cell to which the edge belongs.
    //2 points for one edge.
    //x,y,z for 6 points.
    double edgesPoints[6][3] = {
        {0.8, 1, 1},        //point1 of edge1
        {1, 0.8, 1},        //point2 of edge1
        {0.8, 0.8, 1},      //point1 of edge2
        {1, 0.8, 1},        //point2 of edge2
        {0.8, 1, 1},        //point1 of edge3
        {0.8, 0.8, 1}       //point2 of edge3
    };
    pfMesh->createTriangle(idOfCells, 6, edgesPoints);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::projectNodesDemo(const std::string& path)
{
    int type = 0;
    std::cout << "Select one demo for project: " << std::endl;
    std::cout << "#### 1. point to plane. " << std::endl;
    std::cout << "#### 2. point to edge. " << std::endl;
    std::cout << "#### 3. point to point. " << std::endl;
    std::cout << "#### 4. point to line with two point. " << std::endl;
    std::cout << "#### 5. multiple point to plane. " << std::endl;

    std::cin >> type;
    switch (type)
    {
    case 1:
        projectNodesToPlaneDemo(path);
        break;
    case 2:
        projectNodesToEdgeDemo(path);
        break;
    case 3:
        projectNodesToPointDemo(path);
        break;
    case 4:
        projectNodesToLineDemo(path);
        break;
    case 5:
        projectNodesMoreToPlaneDemo(path);
        break;
    default:
        break;
    }
}

void SurfaceMeshEditDemo::projectNodesToPlaneDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    //id of cell.
    int idOfCells[1] = { 43 };//The id of the cell to which the point belongs.
    //coordinate of point.
    double points[1][3] = { {0.47375109791755676,0.50059449672698975,1} };
    //a point in the plane.
    double pointInPlane[3] = { 0.28899604082107544, 0.59149813652038574, 0 };
    //normal vector of plane.
    double normalVector[3] = { 0, 0, 1 };
    

    pfMesh->projectToPlane(1, idOfCells, points, pointInPlane, normalVector);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::projectNodesToEdgeDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    //id of cell.
    int idOfCells[1] = { 61 };//The id of the cell to which the point belongs.
    //coordinate of point.
    double points[1][3] = { {0.69092792272567749, 0.50051838159561157, 1} };
    //a point in the edge.
    double pointInEdge[3] = { 0, 0.5, 0 };
    // vector of edge.
    double lineVector[3] = { 0, 1, 0 };

    pfMesh->projectToEdge(1, idOfCells, points, pointInEdge, lineVector);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::projectNodesToPointDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    //id of cell.
    int idOfCells[1] = { 42 };//The id of the cell to which the point belongs.
    //coordinate of point.
    double points[1][3] = { {0.47375109791755676,0.50059449672698975,1} };
    //target point of projection.
    double targetPoint[3] = { 0.13664658367633820,0.86525523662567139,0 };
    
    pfMesh->projectToPoint(1, idOfCells, points, targetPoint);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::projectNodesToLineDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    //id of cell.
    int idOfCells[1] = { 35 };//The id of the cell to which the point belongs.
    //coordinate of point with id 35.
    double points[1][3] = { {0.69291967153549194,0.84462225437164307,1} };
    //one point of the projected target line segment.
    double startPoint[3] = { 0,0,0 };
    //another point of the projected target line segment.
    double endPoint[3] = { 0,1,0 };

    pfMesh->projectToLineSegment(1, idOfCells, points, startPoint, endPoint);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::projectNodesMoreToPlaneDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    //id of cell.
    int idOfCells[3] = { 5, 36, 11 };//The id of the cell to which the point belongs.
    //coordinate of points.
    double points[3][3] = {
        {0.29511249065399170,0.85275667905807495,1},              //point in the cell with id 5.
        {0.81415820121765137,0.68385630846023560,1},              //point in the cell with id 36.
        {0.37418347597122192,0.31923958659172058,1}               //point in the cell with id 11.
    };
    //a point in the plane.
    double pointInPlane[3] = { 0.5, 0.5, 0 };
    //normal vector of plane.
    double normalVector[3] = { 0, 0, 1 };

    pfMesh->projectToPlane(3, idOfCells, points, pointInPlane, normalVector);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::moveToGroupDemo(const std::string& path)
{
    std::string moveToGroupName = "top";
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    std::set<std::string> showGroups;
    showGroups.insert(moveToGroupName);
    //show origin "top".
    RenderDemo::show(pfData, VisualizationType::EMesh, showGroups);
    //move 4 cell of "front" to "top".
    int idOfCells[4] = { 16, 15, 20, 21 };
    pfMesh->moveToGroup(moveToGroupName.c_str(), 4, idOfCells);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    //show new "top"
    RenderDemo::show(pfData2, VisualizationType::EMesh, showGroups);

}

void SurfaceMeshEditDemo::intersectDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh3.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    std::set<std::string> showGroups;
    showGroups.insert("back");
    showGroups.insert("left");
    //show origin.
    RenderDemo::show(pfData, VisualizationType::EMesh, showGroups);
    int idOfCells[12] = { 284,313,270,102,120,101,98,91,121,268,310,276 };
    pfMesh->intersect(12, idOfCells);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    //show new.
    RenderDemo::show(pfData2, VisualizationType::EMesh, showGroups);
}

void SurfaceMeshEditDemo::fillHolesDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
    // 1 hole has >= 3 edges. There can be one or more holes.
    // The demo use 3 edges.
    int idOfCells[3] = { 0, 8, 4 };   //The id of the cell to which the edge belongs.
    // 1 edge = 2 points.
    double points[6][3] = {
        {0.8, 1, 1},    //point1 of edge1
        {1, 0.8, 1},    //point2 of edge1
        {0.8, 0.8, 1},  //point1 of edge2
        {1, 0.8, 1},    //point2 of edge2
        {0.8, 1, 1},    //point1 of edge3
        {0.8, 0.8, 1}   //point2 of edge3
    };
    pfMesh->fillHoles(3,idOfCells, 6, points);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::splitDemo(const std::string& path)
{
    int type = 0;
    std::cout << "Select one demo for split: " << std::endl;
    std::cout << "#### 1. Split edges. " << std::endl;
    std::cout << "#### 2. Split cells. " << std::endl;

    std::cin >> type;
    switch (type)
    {
    case 1:
        splitEdgesDemo(path);
        break;
    case 2:
        splitCellsDemo(path);
        break;
    default:
        break;
    }
}

void SurfaceMeshEditDemo::splitEdgesDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");

    int idOfCells[2] = { 22,42 };   //The id of the cell to which the edge belongs.
    int indexOfEdges[2] = { 1,2 };  //The index of the edge in the cell mentioned above.

    pfMesh->splitEdges(2, idOfCells, indexOfEdges);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::splitCellsDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");

    int idOfCells[2] = { 22,49 };   //The id of the cells to be split.

    pfMesh->splitCells(2, idOfCells);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::smoothCellsDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh4.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
    
    int idOfCells[5] = { 42,39,38,20,22 };   //The id of the cell to be smoothed.
    // demo use triangle mesh,  1 cell has 3 point, 5*3 =15.
    double points[15][3] = {
        {0.478805,0.322204,1},
        {0.374183,0.31924,1},
        {0.22348,0.500368,1},
        {0.478805,0.322204,1},
        {0.22348,0.500368,1},
        {0.37423,0.681716,1},
        {0.478805,0.322204,1},
        {0.37423,0.681716,1},
        {0.589717,0.675676,1},
        {0.690928,0.500518,1},
        {0.478805,0.322204,1},
        {0.589717,0.675676,1},
        {0.589674,0.325344,1},
        {0.478805,0.322204,1},
        {0.690928,0.500518,1},
    };
    PREPRO_MESH_NAMESPACE::PFSurfaceSmoothParameter smoothParameter;
    smoothParameter.iterations = 2;
    smoothParameter.perserveEdge = true;
    smoothParameter.projectSurface = true;
    smoothParameter.numberOfCells = 5;
    smoothParameter.idOfCells = idOfCells;
    smoothParameter.numberOfPoints = 15;
    smoothParameter.points = points;

    pfMesh->smoothCells(smoothParameter);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::swapEdgeDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh2.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    int idOfCell = 38;   //The id of the cell to which the edge belongs.
    int indexOfEdge = 0;  //The index of the edge in the cell mentioned above.

    pfMesh->swapEdge(idOfCell, indexOfEdge);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::remeshCellsDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh4.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    int idOfCells[62];   //The id of the cells to be remeshed.
    for (int i = 0; i < 62; ++i)
    {
        idOfCells[i] = i;   //front group id = [0,61];
    }

    pfMesh->remeshCells(62, idOfCells);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::collapseEdgesDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);
    // This demo use 2 edges. You can use one or more edges.
    int idOfCells[2] = { 17, 14};   //The id of the cell to which the edge belongs.
    // 1 edge = 2 points.
    double points[4][3] = {
        {0.2, 0.4, 1},    //point1 of edge1
        {0.2, 0.6, 1},    //point2 of edge1
        {0.8, 0.6, 1},    //point1 of edge2
        {0.8, 0.4, 1}     //point2 of edge2
    };
    pfMesh->collapseEdges(2, idOfCells, 4, points);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    RenderDemo::show(pfData2, VisualizationType::EMesh);
}

void SurfaceMeshEditDemo::deleteCellsDemo(const std::string& path)
{
    OPEN_DOCUMENT(path + "mesh\\surfaceMesh.ppcf");
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    std::set<std::string> showGroups;
    showGroups.insert("front");
    //show origin.
    RenderDemo::show(pfData, VisualizationType::EMesh, showGroups);
    int idOfCells[4] = { 6,7,11,12 };
    pfMesh->deleteCells(4, idOfCells);
    PREPRO_BASE_NAMESPACE::PFData pfData2;
    pfMesh->getAllData(pfData2);
    //show new.
    RenderDemo::show(pfData2, VisualizationType::EMesh, showGroups);
}
