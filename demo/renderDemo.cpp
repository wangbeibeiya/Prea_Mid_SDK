// Revision: $Id: renderDemo.cpp,v 1.0 2025/02/26 11:01:36 xiaming Exp $
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
#include "renderDemo.h"

#include <base/pfGroupData.h>
#include <postProcess/pfPostProcessVariableValues.h>
#include <mesh/pfMeshLeakPath.h>

#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkAutoInit.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkRendererCollection.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkCompositePolyDataMapper2.h>
#include <vtkCamera.h>
#include <base/pfGroupData.h>

vtkSmartPointer<vtkActor> buildVTKGroup(PREPRO_BASE_NAMESPACE::PFGroup* currentGroup, VisualizationType type)
{
    std::string groupName = currentGroup->getName();
    unsigned int groupId = currentGroup->getId();

    // get all elements of current group
    size_t elementSize = currentGroup->getElementSize();
    if (elementSize < 1)
    {
        return nullptr;
    }
    PREPRO_BASE_NAMESPACE::PFElement** currentElements = currentGroup->getElements();

    vtkSmartPointer<vtkPolyData> demoData = vtkSmartPointer<vtkPolyData>::New();
    demoData->Allocate(elementSize);

    VTKCellType cellType = VTKCellType::VTK_TRIANGLE;
    PREPRO_BASE_NAMESPACE::PFElement* currentElement = nullptr;
    unsigned int elementVertexSize = 0;
    unsigned int* elementVertexes = nullptr;
    for (int j = 0; j < elementSize; j++)
    {
        currentElement = currentElements[j];

        elementVertexSize = currentElement->getVertexSize();
        elementVertexes = currentElement->getVertexes();

        vtkIdType* vertexIds = new vtkIdType[elementVertexSize];

        for (unsigned int k = 0; k < elementVertexSize; k++)
        {
            vertexIds[k] = elementVertexes[k];
        }

        if (elementVertexSize == 1)
        {
            cellType = VTKCellType::VTK_VERTEX;
            demoData->InsertNextCell(VTK_VERTEX, 1, vertexIds);
        }
        else if (elementVertexSize == 2)
        {
            cellType = VTKCellType::VTK_LINE;
            demoData->InsertNextCell(VTK_LINE, 2, vertexIds);
        }
        else if (elementVertexSize == 3)
        {
            demoData->InsertNextCell(VTK_TRIANGLE, 3, vertexIds);
        }
        else if (elementVertexSize == 4)
        {
            demoData->InsertNextCell(VTK_QUAD, 4, vertexIds);
        }
        else if (elementVertexSize > 4)
        {
            demoData->InsertNextCell(VTK_POLYGON, static_cast<int>(elementVertexSize), vertexIds);
        }
    }

    // get vertexes of current group
    size_t groupVertexSize = currentGroup->getVertexSize();
    double* groupVertexes = currentGroup->getVertexes();
    if (groupVertexSize < 1)
    {
        return nullptr;
    }

    vtkSmartPointer<vtkFloatArray> floatArray = vtkSmartPointer<vtkFloatArray>::New();
    floatArray->SetNumberOfComponents(3);
    floatArray->SetNumberOfTuples(groupVertexSize);
    int startIndex = 0;
    for (int j = 0; j < groupVertexSize; j++)
    {
        startIndex = 3 * j;
        floatArray->SetTuple3(j, groupVertexes[startIndex], groupVertexes[startIndex + 1], groupVertexes[startIndex + 2]);
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetData(floatArray);

    demoData->SetPoints(points);

    vtkSmartPointer<vtkPolyDataMapper> demoMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    demoMapper->SetInputData(demoData);

    vtkSmartPointer<vtkActor> demoActor = vtkSmartPointer<vtkActor>::New();
    demoActor->SetMapper(demoMapper);
    if (cellType == VTKCellType::VTK_VERTEX)
    {
        demoActor->GetProperty()->SetColor(1.0, 0.0, 0.0); // red
        if (type == VisualizationType::EPostProcess)
        {
            demoActor->GetProperty()->SetPointSize(10);
            demoActor->GetProperty()->SetRenderPointsAsSpheres(1);
            demoActor->GetProperty()->SetColor(1.0, 1.0, 1.0); 
        }
    }
    else if (cellType == VTKCellType::VTK_LINE)
    {
        demoActor->GetProperty()->SetColor(0.0, 1.0, 0.0); // green
    }

    if (groupName == ".densities")
    {
        demoActor->GetProperty()->SetRepresentationToWireframe();
    }

    if (type == VisualizationType::EMesh)
    {
        demoActor->GetProperty()->EdgeVisibilityOn();
        demoActor->GetProperty()->SetEdgeColor(0, 0, 0); // set edge color to black
    }
    return demoActor;
}
void RenderDemo::show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType type, std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupList)
{
    if (0 == shownData.getGroupSize() && 0 == shownData.getVolumeSize())
    {
        return;
    }

    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    // single groups
    std::string checkName;
    PREPRO_BASE_NAMESPACE::PFGroup* currentGroup = nullptr;
    unsigned int groupSize = shownData.getGroupSize();
    PREPRO_BASE_NAMESPACE::PFGroup** singleGroups = shownData.getGroups();
    for (unsigned int i = 0; i < groupSize; i++)
    {
        currentGroup = singleGroups[i];
        checkName = currentGroup->getName();

        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        demoRender->AddActor(groupActor);
    }

    // volumes
    unsigned int volumeSize = shownData.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = shownData.getVolumes();
    PREPRO_BASE_NAMESPACE::PFGroup** volumeGroups = nullptr;
    for (unsigned int i = 0; i < volumeSize; i++)
    {
        PREPRO_BASE_NAMESPACE::PFVolume* volume = volumes[i];
        checkName = volume->getName();
        volumeGroups = volume->getGroups();
        for (unsigned int j = 0; j < volume->getGroupSize(); j++)
        {
            currentGroup = volumeGroups[j];
            checkName = currentGroup->getName();
            vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
            demoRender->AddActor(groupActor);
        }
    }

    for (unsigned int i = 0; i < groupList.size(); ++i)
    {
        currentGroup = groupList[i];
        checkName = currentGroup->getName();

        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        demoRender->AddActor(groupActor);
    }


    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);

    demoInteractor->Initialize();
    demoInteractor->Start();
}

void RenderDemo::show(const PREPRO_BASE_NAMESPACE::PFGroup& pfGroup, VisualizationType type)
{
    if (pfGroup.getVertexSize() == 0 || pfGroup.getElementSize() == 0)
    {
        return;
    }
    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    PREPRO_BASE_NAMESPACE::PFGroup* group = const_cast<PREPRO_BASE_NAMESPACE::PFGroup*>(&pfGroup);
    vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(group, type);
    demoRender->AddActor(groupActor);


    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);

    demoInteractor->Initialize();
    demoInteractor->Start();
}

vtkSmartPointer<vtkMultiBlockDataSet>  buildPostProcessGroupDataset(const PREPRO_BASE_NAMESPACE::PFData& shownData, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues& variableValues,
    double valueRange[2])
{
    unsigned int groupSize = shownData.getGroupSize();
    if (groupSize < 1 || groupSize != variableValues.getGroupSize())
    {
        return nullptr;
    }
    valueRange[0] = VTK_DOUBLE_MAX;
    valueRange[1] = VTK_DOUBLE_MIN;
    PREPRO_BASE_NAMESPACE::PFGroup* currentGroup = nullptr;
    PREPRO_BASE_NAMESPACE::PFGroup** groups = shownData.getGroups();

    vtkSmartPointer<vtkMultiBlockDataSet> multiBlockDataset= vtkSmartPointer<vtkMultiBlockDataSet>::New();
    multiBlockDataset->SetNumberOfBlocks(groupSize);
    for (unsigned int i = 0; i < groupSize; i++)
    {
        currentGroup = groups[i];
        // get all elements of current group
        size_t elementSize = currentGroup->getElementSize();
        if (elementSize < 1)
        {
            return nullptr;
        }
        PREPRO_BASE_NAMESPACE::PFElement** currentElements = currentGroup->getElements();

        vtkSmartPointer<vtkPolyData> demoData = vtkSmartPointer<vtkPolyData>::New();
        demoData->Allocate(elementSize);

        VTKCellType cellType = VTKCellType::VTK_TRIANGLE;
        PREPRO_BASE_NAMESPACE::PFElement* currentElement = nullptr;
        unsigned int elementVertexSize = 0;
        unsigned int* elementVertexes = nullptr;
        for (int j = 0; j < elementSize; j++)
        {
            currentElement = currentElements[j];

            elementVertexSize = currentElement->getVertexSize();
            elementVertexes = currentElement->getVertexes();

            vtkIdType* vertexIds = new vtkIdType[elementVertexSize];

            for (unsigned int k = 0; k < elementVertexSize; k++)
            {
                vertexIds[k] = elementVertexes[k];
            }

            if (elementVertexSize == 1)
            {
                cellType = VTKCellType::VTK_VERTEX;
                demoData->InsertNextCell(VTK_VERTEX, 1, vertexIds);
            }
            else if (elementVertexSize == 2)
            {
                cellType = VTKCellType::VTK_LINE;
                demoData->InsertNextCell(VTK_LINE, 2, vertexIds);
            }
            else if (elementVertexSize == 3)
            {
                demoData->InsertNextCell(VTK_TRIANGLE, 3, vertexIds);
            }
            else if (elementVertexSize == 4)
            {
                demoData->InsertNextCell(VTK_QUAD, 4, vertexIds);
            }
            else if (elementVertexSize > 4)
            {
                demoData->InsertNextCell(VTK_POLYGON, static_cast<int>(elementVertexSize), vertexIds);
            }
        }

        // get vertexes of current group
        size_t groupVertexSize = currentGroup->getVertexSize();
        double* groupVertexes = currentGroup->getVertexes();
        if (groupVertexSize < 1)
        {
            return nullptr;
        }

        vtkSmartPointer<vtkFloatArray> floatArray = vtkSmartPointer<vtkFloatArray>::New();
        floatArray->SetNumberOfComponents(3);
        floatArray->SetNumberOfTuples(groupVertexSize);
        int startIndex = 0;
        for (int j = 0; j < groupVertexSize; j++)
        {
            startIndex = 3 * j;
            floatArray->SetTuple3(j, groupVertexes[startIndex], groupVertexes[startIndex + 1], groupVertexes[startIndex + 2]);
        }
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        points->SetData(floatArray);
        demoData->SetPoints(points);
        multiBlockDataset->SetBlock(i, demoData);
        //variable data
        if (variableValues.getGroupSize(i) == 0)
        {
            continue;
        }
        //create vtkFloatArray to store array data
        vtkSmartPointer<vtkFloatArray> activeScalarArray = vtkSmartPointer<vtkFloatArray>::New();
        activeScalarArray->SetNumberOfComponents(1);
        activeScalarArray->SetNumberOfTuples(variableValues.getGroupSize(i));

        for (vtkIdType pointDataIndex = 0; pointDataIndex < variableValues.getGroupSize(i); ++pointDataIndex)
        {
            activeScalarArray->SetValue(pointDataIndex, variableValues.getGroupValues(i)[pointDataIndex]);
        }
        demoData->GetPointData()->AddArray(activeScalarArray);
        demoData->GetPointData()->SetScalars(activeScalarArray);
        //get range and min max value
        double range[2];
        activeScalarArray->GetRange(range); 
        if (range[0] < valueRange[0])
        {
            valueRange[0] = range[0];
        }
        if (range[1] > valueRange[1])
        {
            valueRange[1] = range[1];
        }
    }
    return multiBlockDataset;
}
void RenderDemo::showResult(const PREPRO_BASE_NAMESPACE::PFData& shownData, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues& variableValues)
{
    if (0 == shownData.getGroupSize() && 0 == shownData.getVolumeSize())
    {
        return;
    }

    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    double range[2] = {0.};
    vtkSmartPointer<vtkMultiBlockDataSet> multiBlcokDataset = buildPostProcessGroupDataset(shownData, variableValues, range);
    vtkSmartPointer<vtkCompositePolyDataMapper2> demoMapper = vtkSmartPointer<vtkCompositePolyDataMapper2>::New();
    demoMapper->SetInputDataObject(multiBlcokDataset);
    demoMapper->SetScalarModeToUsePointData();
    demoMapper->SetScalarRange(range);

    vtkSmartPointer<vtkActor> demoActor = vtkSmartPointer<vtkActor>::New();
    demoActor->SetMapper(demoMapper);
    demoRender->AddActor(demoActor);
    demoActor->GetProperty()->SetPointSize(10);
    demoActor->GetProperty()->SetRenderPointsAsSpheres(1);

    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->SetTableRange(range);//set table range as same as data's range
    lookupTable->SetHueRange(0.667, 0.0); //blue to red
    lookupTable->SetNanColor(0.5, 0.5, 0.5, 1.); //the color to use when a NaN(not a number) is encountered.
    lookupTable->Build();

    demoMapper->SetLookupTable(lookupTable);
    demoMapper->Update();

    demoMapper->InterpolateScalarsBeforeMappingOn();
    demoMapper->SetResolveCoincidentTopologyToPolygonOffset();

    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);
	//set view: parallel to the YZ-plane, looking from the positive X-axis.
	vtkCamera* camera = demoRender->GetActiveCamera();
	if (nullptr != camera)
	{
		camera->SetFocalPoint(0, 0, 0);
		camera->SetPosition(1, 0, 0);
		camera->SetViewUp(0.0, 1.0, 0.0);
		demoRender->ResetCamera();
	}
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);
    demoInteractor->Initialize();
    demoInteractor->Start();
}

void RenderDemo::showLeakPath(const PREPRO_MESH_NAMESPACE::PFLeakPathContainer& leakPath)
{
    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> leakpathRender = vtkSmartPointer<vtkRenderer>::New();
    leakpathRender->SetBackground(0.1, 0.2, 0.4);

    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    // set render data from leakPath.
    vtkIdType pointOffset = 0;
    int pathSize = 0;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < leakPath.getSize(); ++i)
    {
        PREPRO_MESH_NAMESPACE::PFLeakPathPointArray* pathArray = leakPath.getArray(i);
        int arraySize = pathArray->getSize();
        vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
        polyLine->GetPointIds()->SetNumberOfIds(arraySize);

        // use different color for each polyline.
        unsigned char color[3];
        color[0] = (pathSize + 50) % 255;
        color[1] = (pathSize + 100) % 255;
        color[2] = (pathSize + 200) % 255;

        for (int j = 0; j < arraySize; ++j)
        {
            points->InsertNextPoint(pathArray->getPoint(j));
            polyLine->GetPointIds()->SetId(j, pointOffset + j);
            colors->InsertNextTypedTuple(color);
        }

        cells->InsertNextCell(polyLine);

        pointOffset += arraySize;
        pathSize += 90;
    }

    polyData->SetPoints(points);
    polyData->SetLines(cells);
    polyData->GetPointData()->SetScalars(colors);
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> leakpathActor = vtkSmartPointer<vtkActor>::New();
    leakpathActor->SetMapper(mapper);

    vtkSmartPointer<vtkRenderWindow> leakpathWindow = vtkSmartPointer<vtkRenderWindow>::New();
    leakpathWindow->AddRenderer(leakpathRender);
    leakpathWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> leakpathInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    leakpathInteractor->SetRenderWindow(leakpathWindow);

    leakpathRender->AddActor(leakpathActor);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> leakpathStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    leakpathInteractor->SetInteractorStyle(leakpathStyle);

    leakpathInteractor->Initialize();
    leakpathInteractor->Start();
}

void RenderDemo::show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType type, std::set<std::string> groupNames)
{
    if (0 == shownData.getGroupSize() && 0 == shownData.getVolumeSize())
    {
        return;
    }

    auto isShowGroup = [&groupNames](std::string name)->bool
    {
        auto it = groupNames.find(name);
        return it != groupNames.end();
    };

    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    // single groups
    std::string checkName;
    PREPRO_BASE_NAMESPACE::PFGroup* currentGroup = nullptr;
    unsigned int groupSize = shownData.getGroupSize();
    PREPRO_BASE_NAMESPACE::PFGroup** singleGroups = shownData.getGroups();
    for (unsigned int i = 0; i < groupSize; i++)
    {
        currentGroup = singleGroups[i];
        checkName = currentGroup->getName();
        if (!isShowGroup(checkName))
        {
            continue;
        }

        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        demoRender->AddActor(groupActor);
    }

    // volumes
    unsigned int volumeSize = shownData.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = shownData.getVolumes();
    PREPRO_BASE_NAMESPACE::PFGroup** volumeGroups = nullptr;
    for (unsigned int i = 0; i < volumeSize; i++)
    {
        PREPRO_BASE_NAMESPACE::PFVolume* volume = volumes[i];
        checkName = volume->getName();
        volumeGroups = volume->getGroups();
        for (unsigned int j = 0; j < volume->getGroupSize(); j++)
        {
            currentGroup = volumeGroups[j];
            checkName = currentGroup->getName();
            if (!isShowGroup(checkName))
            {
                continue;
            }
            vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
            demoRender->AddActor(groupActor);
        }
    }

    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);

    demoInteractor->Initialize();
    demoInteractor->Start();
}

void RenderDemo::show(const PREPRO_BASE_NAMESPACE::PFVolume& pfVolume, VisualizationType type)
{
    if (pfVolume.getGroupSize() == 0 || pfVolume.getGroups() == nullptr)
    {
        return;
    }
    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    PREPRO_BASE_NAMESPACE::PFGroup* currentGroup = nullptr;
    std::string checkName;
    PREPRO_BASE_NAMESPACE::PFGroup** volumeGroups = pfVolume.getGroups();
    for (unsigned int j = 0; j < pfVolume.getGroupSize(); j++)
    {
        currentGroup = volumeGroups[j];
        checkName = currentGroup->getName();
        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        demoRender->AddActor(groupActor);
    }
    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);

    demoInteractor->Initialize();
    demoInteractor->Start();
}