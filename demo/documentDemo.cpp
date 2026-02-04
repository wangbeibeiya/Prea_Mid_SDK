// Revision: $Id: saveDocumentDemo.cpp,v 1.0 2025/02/21 14:55:04 Leon Exp $
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
#include "documentDemo.h"

#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <base/pfGroupData.h>
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometry.h>
#include <geometry/pfGeometryBuilder.h>
#include <mesh/pfMesh.h>
#include <mesh/pfMeshBuilder.h>
#include "renderDemo.h"
#include "commonMethods.h"

#include <iostream>
using namespace std;
using namespace PREPRO_BASE_NAMESPACE;
using namespace PREPRO_GEOMETRY_NAMESPACE;

#define MAX_TIME_LENGTH 256

std::string applicationPath;

/*!
* \fn get the timestamp of file generated.
*/
void getFootprint(char* buffer)
{
    std::time_t now = std::time({});
    std::tm tm = *std::localtime(&now);
    std::strftime(buffer, MAX_TIME_LENGTH, "%FT%H%M%SZ", &tm);
}

void DocumentDemo::documentDemo(const string& examplePath)
{
    applicationPath = examplePath;

    std::cout << "##### Demos, please select a number for document test demo: " << std::endl;
    std::cout << "#####   1 - Save As (new document then import geometry) alone." << std::endl;
    std::cout << "#####   2 - Save As (new document then import mesh) alone." << std::endl;
    std::cout << "#####   3 - Save As (new document then import geometry and mesh)." << std::endl;
    std::cout << "#####   4 - Save As (new document is empty)." << std::endl;
    std::cout << "#####   5 - others." << std::endl;
    std::cout << "input number ->  " << std::endl;

    int type = 0;
    std::cin >> type;
    switch (type)
    {
    case 1:
        testSaveAsNewGeometryAlone(examplePath);
        return;
    case 2:
        testSaveAsNewMeshAlone(examplePath);
        return;
    case 3:
        testSaveAsNewGeometryAndMesh(examplePath);
        return;
    case 4:
        testSaveAsNewEmpty(examplePath);
        return;
    default:
    case 5:
        break;
    }

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

    std::cout << "### Command working folder is: " << examplePath << std::endl;
    // 2. load document example file.
    std::string exampleFilePath = examplePath + "testdoc.ppcf";
    std::cout << "### Open ppcf path is: " << exampleFilePath << std::endl;

    PFDocument* pfDocument = pfApplication.openDocument(exampleFilePath.c_str());
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

    // 3. get geometry data from current document.
    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfGeometry->getAllData(pfData);

    // 4. render
    RenderDemo::show(pfData, VisualizationType::EGeomerty);

    // 5. after the render is done, we can save document and close it.
    saveDocument(&pfApplication, pfDocument);

    // 6. close it. since we already have saved, we just close without saving again.
    closeDocument(&pfApplication, pfDocument, false);
}

PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::testSaveAsNewGeometryAlone(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            std::cout << "No PERA SIM license" << std::endl;
            return PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        std::cout << "Current Application has no geometry privilege" << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // set import option for group mode.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromSurfaceName);

    // import first geometry 
    std::string geometryPath = examplePath + "import\\HeatSink_cf.stl";
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->importGeometry(geometryPath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Import Geometry failed: => " << geometryPath << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::string savePPCFName;
    size_t found = geometryPath.find_last_of("/\\");
    if (found != std::string::npos) {
        savePPCFName = examplePath + geometryPath.substr(found + 1) + ".ppcf";
    }

    PREPRO_BASE_NAMESPACE::PFStatus isOK;
    pfDocument->setDocumentPath(savePPCFName.c_str());
    isOK = pfDocument->save();
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save document is failed." << std::endl;
        return isOK;
    }

    // save as
    char buffer[MAX_TIME_LENGTH];
    getFootprint(&buffer[0]);
    std::string saveAsFileName = applicationPath + "SDKDemo_saveAs_NewGeometryAlone_";
    saveAsFileName += buffer;
    saveAsFileName += ".ppcf";
    isOK = pfDocument->saveAs(saveAsFileName.c_str());
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save As document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save As document is failed." << std::endl;
        return isOK;
    }

    // close document.
    pfApplication.closeDocument(pfDocument, false);

    // test open the document of the saved(as) document.
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocumentAgain = pfApplication.openDocument(saveAsFileName.c_str());

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometryAgain = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocumentAgain->getGeometryEnvironment());
    if (nullptr == geometryAgain)
    {
        std::cout << "\n\nCurrent Application has no geometry privilege.\n\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;

    geometryAgain->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EGeomerty);

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::testSaveAsNewMeshAlone(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (meshBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            std::cout << "No PERA SIM license" << std::endl;
            return PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();

    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::string meshFilePath = examplePath + "onlymesh.ppcf";
    PREPRO_BASE_NAMESPACE::PFStatus status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Import mesh file failed:" << meshFilePath << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFStatus isOK;
    pfDocument->setDocumentPath(meshFilePath.c_str());
    isOK = pfDocument->save();
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save document is failed." << std::endl;
        return isOK;
    }

    // save as
    char buffer[MAX_TIME_LENGTH];
    getFootprint(&buffer[0]);
    std::string saveAsFileName = applicationPath + "SDKDemo_saveAs_NewMeshAlone_";
    saveAsFileName += buffer;
    saveAsFileName += ".ppcf";
    isOK = pfDocument->saveAs(saveAsFileName.c_str());
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save As document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save As document is failed." << std::endl;
    }

    // close document.
    pfApplication.closeDocument(pfDocument, false);

    // test open the document of the saved(as) document.
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocumentAgain = pfApplication.openDocument(saveAsFileName.c_str());

    PREPRO_MESH_NAMESPACE::PFMesh* pfMeshAgain = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocumentAgain->getMeshEnvironment());
    if (nullptr == pfMeshAgain)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;

    pfMeshAgain->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    return isOK;
}

PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::testSaveAsNewGeometryAndMesh(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            std::cout << "No PERA SIM license" << std::endl;
            return PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense;
        }
    }

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr != meshBuilder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            std::cout << "No PERA SIM license" << std::endl;
            return PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        std::cout << "\n\nCurrent Application has no geometry privilege\n\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // set import option for group mode.
    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromSurfaceName);

    // import first geometry 
    std::string geometryPath = examplePath + "import\\HeatSink_cf.stl";
    PREPRO_BASE_NAMESPACE::PFStatus status = geometry->importGeometry(geometryPath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Import Geometry failed: => " << geometryPath << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // second mesh file.
    PREPRO_BASE_NAMESPACE::IPFEnvironment* pfMeshEnvironment = pfDocument->getMeshEnvironment();
    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfMeshEnvironment);
    if (nullptr == pfMesh)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::string meshFilePath = examplePath + "onlymesh.ppcf";
    status = pfMesh->importMesh(meshFilePath.c_str(), PREPRO_BASE_NAMESPACE::ImportOptions::EAppend);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Import mesh file OK:" << meshFilePath << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Import mesh file failed:" << meshFilePath << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }


    std::string savePPCFName;
    size_t found = geometryPath.find_last_of("/\\");
    if (found != std::string::npos) {
        savePPCFName = examplePath + geometryPath.substr(found + 1) + ".ppcf";
    }

    PREPRO_BASE_NAMESPACE::PFStatus isOK;
    pfDocument->setDocumentPath(savePPCFName.c_str());
    isOK = pfDocument->save();
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save document is failed." << std::endl;
        return isOK;
    }

    // save as
    char buffer[MAX_TIME_LENGTH];
    getFootprint(&buffer[0]);
    std::string saveAsFileName = applicationPath + "SDKDemo_saveAs_NewGeometryAndMesh_";
    saveAsFileName += buffer;
    saveAsFileName += ".ppcf";
    isOK = pfDocument->saveAs(saveAsFileName.c_str());
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save As document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save As document is failed." << std::endl;
        return isOK;
    }

    // close document.
    pfApplication.closeDocument(pfDocument, false);

    // test open the document of the saved(as) document.
    PREPRO_BASE_NAMESPACE::PFDocument* pfDocumentAgain = pfApplication.openDocument(saveAsFileName.c_str());
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometryAgain = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocumentAgain->getGeometryEnvironment());
    if (nullptr == geometryAgain)
    {
        std::cout << "\n\nCurrent Application has no geometry privilege\n\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMeshAgain = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocumentAgain->getMeshEnvironment());
    if (nullptr == pfMeshAgain)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_BASE_NAMESPACE::PFData pfData;
    geometryAgain->getAllData(pfData);

    RenderDemo::show(pfData, VisualizationType::EGeomerty);

    pfMeshAgain->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::testSaveAsNewEmpty(const std::string& examplePath)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* builder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (builder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(builder))
        {
            std::cout << "No PERA SIM license" << std::endl;
            return PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == geometry)
    {
        std::cout << "\n\nCurrent Application has no geometry privilege.\n\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::string savePPCFName = examplePath + "empty.ppcf";

    PREPRO_BASE_NAMESPACE::PFStatus isOK;
    pfDocument->setDocumentPath(savePPCFName.c_str());
    isOK = pfDocument->save();
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save document is ok." << std::endl;
    }
    else if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EEmptyDocument)
    {
        std::cout << "\n\n=====Save document is failed, because Document is empty. You need to have geometry or mesh inside this document to save.\n\n";
        return isOK;
    }
    else
    {
        std::cout << "\n\n=====Save document is failed." << std::endl;
        return isOK;
    }

    // save as
    char buffer[MAX_TIME_LENGTH];
    getFootprint(&buffer[0]);
    std::string saveAsFileName = applicationPath + "SDKDemo_saveAs_NewEmpty_";
    saveAsFileName += buffer;
    saveAsFileName += ".ppcf";
    isOK = pfDocument->saveAs(saveAsFileName.c_str());
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save As document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save As document is failed." << std::endl;
    }

    return isOK;
}

/*!
* \fn Test case: save document.
*/
PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::saveDocument(PFApplication* pfApplication, PREPRO_BASE_NAMESPACE::PFDocument* pfDocument)
{
    if (pfDocument == nullptr)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    // case 1 - save current document directly.
    PREPRO_BASE_NAMESPACE::PFStatus isOK = pfDocument->save();
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save document is failed." << std::endl;
        return isOK;
    }

    // case 2 - save the document to another path.
    char buffer[MAX_TIME_LENGTH];
    getFootprint(&buffer[0]);
    std::string saveAsFileName = applicationPath + "SDKDemo_saveAs_";
    saveAsFileName += buffer;
    saveAsFileName += ".ppcf";
    isOK = pfDocument->saveAs(saveAsFileName.c_str());
    if (isOK == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\n\n=====Save As document is ok." << std::endl;
    }
    else
    {
        std::cout << "\n\n=====Save As document is failed." << std::endl;
    }

    return isOK;
}

PREPRO_BASE_NAMESPACE::PFStatus DocumentDemo::closeDocument(PFApplication* pfApplication, PFDocument* pfDocument, bool needSave)
{
    return pfApplication->closeDocument(pfDocument, needSave);
}