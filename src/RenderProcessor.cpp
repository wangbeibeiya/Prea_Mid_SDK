// Revision: $Id: RenderProcessor.cpp,v 1.0 2025/02/26 11:01:36 xiaming Exp $
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
#include "RenderProcessor.h"

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
#include <vtkActorCollection.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <base/pfGroupData.h>
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

// 注意：不直接包含 MeshVisualizationServer.h，避免 Windows Socket 头文件冲突
// 使用前向声明和 extern "C" 函数
class MeshVisualizationServer;  // 前向声明

extern "C" {
    void SetRenderSession(vtkRenderWindow*, vtkRenderer*, vtkActorCollection*, vtkRenderWindowInteractor*, vtkActorCollection* edgeActors = nullptr);
    MeshVisualizationServer* GetServerInstance();
    void SetServerInstance(MeshVisualizationServer*);
    bool IsServerRunning(MeshVisualizationServer*);
    void StopServer(MeshVisualizationServer*);
}

// 窗口关闭回调函数：当窗口关闭时停止 Socket 服务器
void OnWindowClose(vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
{
    // VTK 窗口关闭时触发 ExitEvent
    if (eventId == vtkCommand::ExitEvent)
    {
        std::cout << "[RenderProcessor] 检测到窗口关闭事件，准备停止 Socket 服务器..." << std::endl;
        
        // 从全局状态获取服务器实例并停止
        MeshVisualizationServer* server = GetServerInstance();
        if (server && IsServerRunning(server))
        {
            std::cout << "[RenderProcessor] 正在停止 Socket 服务器..." << std::endl;
            StopServer(server);
            std::cout << "[RenderProcessor] Socket 服务器已停止" << std::endl;
        }
    }
}

// 自定义交互样式类，支持键盘快捷键控制网格可视化
class MeshInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MeshInteractorStyle* New();
    vtkTypeMacro(MeshInteractorStyle, vtkInteractorStyleTrackballCamera);

    void SetRenderer(vtkRenderer* renderer) { m_renderer = renderer; }
    void SetActors(vtkActorCollection* actors) 
    { 
        m_actors = actors; 
        m_actorsRef = actors;  // 强引用防止悬空指针
    }
    void SetEdgeActors(vtkActorCollection* edgeActors) 
    { 
        m_edgeActors = edgeActors; 
        m_edgeActorsRef = edgeActors;  // 强引用
    }

    virtual void OnKeyPress() override
    {
        vtkRenderWindowInteractor* rwi = this->GetInteractor();
        std::string key = rwi->GetKeySym();
        bool altPressed = (rwi->GetAltKey() != 0);
        
        // 仅当 Alt 按下时响应自定义快捷键（Alt+S, Alt+W 等）
        if (altPressed && (key == "s" || key == "S"))
        {
            CycleRepresentation();
        }
        else if (altPressed && (key == "w" || key == "W"))
        {
            ToggleWireframe();
        }
        else if (altPressed && (key == "p" || key == "P"))
        {
            TogglePoints();
        }
        else if (altPressed && (key == "e" || key == "E"))
        {
            ToggleEdges();
        }
        else if (altPressed && (key == "t" || key == "T"))
        {
            ToggleTransparency();
        }
        else if (altPressed && (key == "c" || key == "C"))
        {
            ToggleColorByGroup();
        }
        else if (altPressed && (key == "r" || key == "R"))
        {
            ResetCamera();
        }
        else if (altPressed && (key == "h" || key == "H"))
        {
            ToggleHelp();
        }
        else
        {
            vtkInteractorStyleTrackballCamera::OnKeyPress();
        }
    }

    // 重写 OnChar 以阻止 VTK 默认将 'e' 映射为 ExitCallback（否则按 E 会退出程序）
    virtual void OnChar() override
    {
        vtkRenderWindowInteractor* rwi = this->GetInteractor();
        if (rwi->GetControlKey() || rwi->GetAltKey())
        {
            vtkInteractorStyleTrackballCamera::OnChar();
            return;
        }
        int keyCode = rwi->GetKeyCode();
        // 我们的自定义快捷键：不传给父类，避免 'e' 触发 ExitCallback
        if (keyCode == 'e' || keyCode == 'E' || keyCode == 's' || keyCode == 'S' ||
            keyCode == 'w' || keyCode == 'W' || keyCode == 'p' || keyCode == 'P' ||
            keyCode == 't' || keyCode == 'T' || keyCode == 'c' || keyCode == 'C' ||
            keyCode == 'r' || keyCode == 'R' || keyCode == 'h' || keyCode == 'H')
        {
            return;  // 已在 OnKeyPress 中处理，阻止父类 OnChar 的默认行为（如 e->Exit）
        }
        vtkInteractorStyleTrackballCamera::OnChar();
    }

private:
    vtkRenderer* m_renderer = nullptr;
    vtkActorCollection* m_actors = nullptr;
    vtkActorCollection* m_edgeActors = nullptr;  // 边界线框叠加层（替代 EdgeVisibility，避免 VTK 崩溃）
    vtkSmartPointer<vtkActorCollection> m_actorsRef;  // 强引用，防止 actorCollection 被提前释放导致崩溃
    vtkSmartPointer<vtkActorCollection> m_edgeActorsRef;
    vtkTextActor* m_helpText = nullptr;
    bool m_showHelp = false;
    int m_representationMode = 0; // 0=实体, 1=线框, 2=点
    bool m_showEdges = true;
    bool m_transparent = false;
    bool m_colorByGroup = false;
    int m_currentGroupColorIndex = 0;

    void CycleRepresentation()
    {
        if (!m_actors) return;
        
        m_representationMode = (m_representationMode + 1) % 3;
        
        m_actors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = m_actors->GetNextActor()) != nullptr)
        {
            vtkProperty* prop = actor->GetProperty();
            if (m_representationMode == 0)
                prop->SetRepresentationToSurface();
            else if (m_representationMode == 1)
                prop->SetRepresentationToWireframe();
            else if (m_representationMode == 2)
                prop->SetRepresentationToPoints();
        }
        const char* modeStr[] = {"实体", "线框", "点"};
        std::cout << "显示模式: " << modeStr[m_representationMode] << std::endl;
        // 线框/点模式下隐藏边界叠加层，实体模式下根据 m_showEdges 恢复
        if (m_edgeActors)
        {
            m_edgeActors->InitTraversal();
            vtkActor* ea = nullptr;
            while ((ea = m_edgeActors->GetNextActor()) != nullptr)
            {
                ea->SetVisibility((m_representationMode == 0 && m_showEdges) ? 1 : 0);
            }
        }
        if (m_renderer && m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }

    void ToggleWireframe()
    {
        if (!m_actors) return;
        
        m_representationMode = (m_representationMode == 1) ? 0 : 1;
        
        m_actors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = m_actors->GetNextActor()) != nullptr)
        {
            vtkProperty* prop = actor->GetProperty();
            if (m_representationMode == 1)
                prop->SetRepresentationToWireframe();
            else
                prop->SetRepresentationToSurface();
        }
        std::cout << "线框模式: " << (m_representationMode == 1 ? "开启" : "关闭") << std::endl;
        if (m_edgeActors)
        {
            m_edgeActors->InitTraversal();
            vtkActor* ea = nullptr;
            while ((ea = m_edgeActors->GetNextActor()) != nullptr)
            {
                ea->SetVisibility((m_representationMode == 0 && m_showEdges) ? 1 : 0);
            }
        }
        if (m_renderer && m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }

    void TogglePoints()
    {
        if (!m_actors) return;
        
        // 根据第一个 actor 判断当前是否点模式，保持 S/W/P 状态一致
        m_actors->InitTraversal();
        vtkActor* firstActor = m_actors->GetNextActor();
        bool inPointsMode = (firstActor && firstActor->GetProperty()->GetRepresentation() == VTK_POINTS);
        bool switchToPoints = !inPointsMode;
        m_representationMode = switchToPoints ? 2 : 0;
        
        m_actors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = m_actors->GetNextActor()) != nullptr)
        {
            vtkProperty* prop = actor->GetProperty();
            if (switchToPoints)
            {
                prop->SetRepresentationToPoints();
                prop->SetPointSize(5);
            }
            else
            {
                prop->SetRepresentationToSurface();
            }
        }
        std::cout << "点模式: " << (switchToPoints ? "开启" : "关闭") << std::endl;
        if (m_edgeActors)
        {
            m_edgeActors->InitTraversal();
            vtkActor* ea = nullptr;
            while ((ea = m_edgeActors->GetNextActor()) != nullptr)
            {
                ea->SetVisibility((!switchToPoints && m_showEdges) ? 1 : 0);
            }
        }
        if (m_renderer && m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }

    void ToggleEdges()
    {
        // 使用线框叠加层替代 EdgeVisibility，避免 VTK EdgeVisibility 导致崩溃
        if (!m_edgeActors || !m_renderer) return;
        
        vtkRenderWindow* renderWindow = m_renderer->GetRenderWindow();
        if (!renderWindow) return;
        
        m_showEdges = !m_showEdges;
        
        m_edgeActors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = m_edgeActors->GetNextActor()) != nullptr)
        {
            if (actor)
            {
                actor->SetVisibility(m_showEdges ? 1 : 0);
            }
        }
        std::cout << "边界显示: " << (m_showEdges ? "开启" : "关闭") << std::endl;
        renderWindow->Render();
    }

    void ToggleTransparency()
    {
        if (!m_actors) return;
        
        m_transparent = !m_transparent;
        
        m_actors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = m_actors->GetNextActor()) != nullptr)
        {
            vtkProperty* prop = actor->GetProperty();
            prop->SetOpacity(m_transparent ? 0.5 : 1.0);
        }
        std::cout << "透明度: " << (m_transparent ? "50%" : "100%") << std::endl;
        if (m_renderer && m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }

    void ToggleColorByGroup()
    {
        if (!m_actors) return;
        
        m_colorByGroup = !m_colorByGroup;
        
        // 定义一组颜色用于按组着色
        double colors[][3] = {
            {1.0, 0.0, 0.0}, // 红
            {0.0, 1.0, 0.0}, // 绿
            {0.0, 0.0, 1.0}, // 蓝
            {1.0, 1.0, 0.0}, // 黄
            {1.0, 0.0, 1.0}, // 洋红
            {0.0, 1.0, 1.0}, // 青
            {1.0, 0.5, 0.0}, // 橙
            {0.5, 0.0, 1.0}, // 紫
        };
        int numColors = 8;
        
        m_actors->InitTraversal();
        vtkActor* actor = nullptr;
        int colorIndex = 0;
        while ((actor = m_actors->GetNextActor()) != nullptr)
        {
            vtkProperty* prop = actor->GetProperty();
            if (m_colorByGroup)
            {
                prop->SetColor(colors[colorIndex % numColors]);
                colorIndex++;
            }
            else
            {
                prop->SetColor(0.7, 0.7, 0.7); // 恢复默认灰色
            }
        }
        std::cout << "按组着色: " << (m_colorByGroup ? "开启" : "关闭") << std::endl;
        if (m_renderer && m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }

    void ResetCamera()
    {
        if (m_renderer)
        {
            m_renderer->ResetCamera();
            if (m_renderer->GetRenderWindow())
            {
                m_renderer->GetRenderWindow()->Render();
            }
            std::cout << "视图已重置" << std::endl;
        }
    }

    void ToggleHelp()
    {
        if (!m_renderer) return;
        
        m_showHelp = !m_showHelp;
        
        if (m_showHelp)
        {
            if (!m_helpText)
            {
                m_helpText = vtkTextActor::New();
                m_helpText->SetInput(
                    "=== 网格可视化快捷键 ===\n"
                    "Alt+S - 循环切换显示模式：实体表面 / 线框 / 顶点\n"
                    "Alt+W - 切换线框模式（仅线框与实体之间切换）\n"
                    "Alt+P - 切换点模式（仅顶点与实体之间切换）\n"
                    "Alt+E - 切换网格边线显示（实体模式下显示/隐藏边界线）\n"
                    "Alt+T - 切换半透明显示（50%透明度）\n"
                    "Alt+C - 切换按组着色（多组时用不同颜色区分）\n"
                    "Alt+R - 重置相机到默认视角\n"
                    "Alt+H - 显示/隐藏本帮助\n"
                    "Q - 退出程序\n"
                    "\n--- 鼠标操作 ---\n"
                    "左键拖拽 - 旋转视角\n"
                    "右键拖拽 - 缩放\n"
                    "中键拖拽 - 平移"
                );
                vtkTextProperty* prop = m_helpText->GetTextProperty();
                prop->SetFontSize(14);
                prop->SetColor(1.0, 1.0, 1.0);
                prop->SetBackgroundColor(0.0, 0.0, 0.0);
                prop->SetBackgroundOpacity(0.7);
                m_helpText->SetPosition(10, 10);
            }
            m_renderer->AddActor2D(m_helpText);
        }
        else
        {
            if (m_helpText)
            {
                m_renderer->RemoveActor2D(m_helpText);
            }
        }
        
        if (m_renderer->GetRenderWindow())
        {
            m_renderer->GetRenderWindow()->Render();
        }
    }
};

vtkStandardNewMacro(MeshInteractorStyle);

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

    // 不再使用 EdgeVisibility（会导致 VTK 崩溃），改用线框叠加层
    return demoActor;
}

// 从表面 actor 创建边界线框叠加层（仅对三角形/四边形/多边形有效）
// 使用 DeepCopy 避免与表面 actor 共享 polyData，防止 VTK 渲染时崩溃
static vtkSmartPointer<vtkActor> createEdgeOverlayActor(vtkActor* sourceActor)
{
    if (!sourceActor) return nullptr;
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(sourceActor->GetMapper());
    if (!mapper) return nullptr;
    vtkPolyData* srcPolyData = mapper->GetInput();
    if (!srcPolyData || srcPolyData->GetNumberOfCells() == 0) return nullptr;
    
    vtkSmartPointer<vtkPolyData> edgePolyData = vtkSmartPointer<vtkPolyData>::New();
    edgePolyData->DeepCopy(srcPolyData);
    
    vtkSmartPointer<vtkPolyDataMapper> edgeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    edgeMapper->SetInputData(edgePolyData);
    
    vtkSmartPointer<vtkActor> edgeActor = vtkSmartPointer<vtkActor>::New();
    edgeActor->SetMapper(edgeMapper);
    edgeActor->GetProperty()->SetRepresentationToWireframe();
    edgeActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    edgeActor->GetProperty()->SetLineWidth(1.0);
    edgeActor->SetVisibility(1);  // 默认显示边界
    return edgeActor;
}
void RenderProcessor::show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType type, std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupList)
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

    // 收集所有actors，用于网格可视化控制
    vtkSmartPointer<vtkActorCollection> actorCollection = vtkSmartPointer<vtkActorCollection>::New();
    vtkSmartPointer<vtkActorCollection> edgeActorCollection = vtkSmartPointer<vtkActorCollection>::New();

    auto addActorWithEdgeOverlay = [&](vtkActor* groupActor) {
        if (!groupActor) return;
        demoRender->AddActor(groupActor);
        actorCollection->AddItem(groupActor);
        if (type == VisualizationType::EMesh)
        {
            vtkSmartPointer<vtkActor> edgeActor = createEdgeOverlayActor(groupActor);
            if (edgeActor)
            {
                demoRender->AddActor(edgeActor);
                edgeActorCollection->AddItem(edgeActor);
            }
        }
    };

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
        addActorWithEdgeOverlay(groupActor);
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
            addActorWithEdgeOverlay(groupActor);
        }
    }

    for (unsigned int i = 0; i < groupList.size(); ++i)
    {
        currentGroup = groupList[i];
        checkName = currentGroup->getName();

        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        addActorWithEdgeOverlay(groupActor);
    }

    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(800, 600);
    
    // 设置窗口标题
    if (type == VisualizationType::EMesh)
    {
        demoWindow->SetWindowName("网格可视化 - 按Alt+H查看帮助");
    }
    else if (type == VisualizationType::EGeomerty)
    {
        demoWindow->SetWindowName("几何可视化");
    }
    else
    {
        demoWindow->SetWindowName("可视化");
    }

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    // 如果是网格类型，使用自定义交互样式
    if (type == VisualizationType::EMesh)
    {
        vtkSmartPointer<MeshInteractorStyle> meshStyle = vtkSmartPointer<MeshInteractorStyle>::New();
        meshStyle->SetRenderer(demoRender);
        meshStyle->SetActors(actorCollection);
        meshStyle->SetEdgeActors(edgeActorCollection);
        demoInteractor->SetInteractorStyle(meshStyle);
        
        // 输出帮助信息到控制台
        std::cout << "\n=== 网格可视化控制 ===" << std::endl;
        std::cout << "快捷键:" << std::endl;
        std::cout << "  Alt+S - 切换显示模式(实体/线框/点)" << std::endl;
        std::cout << "  Alt+W - 切换线框模式" << std::endl;
        std::cout << "  Alt+P - 切换点模式" << std::endl;
        std::cout << "  Alt+E - 切换边界显示" << std::endl;
        std::cout << "  Alt+T - 切换透明度" << std::endl;
        std::cout << "  Alt+C - 切换按组着色" << std::endl;
        std::cout << "  Alt+R - 重置视图" << std::endl;
        std::cout << "  Alt+H - 显示/隐藏帮助" << std::endl;
        std::cout << "====================\n" << std::endl;
    }
    else
    {
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        demoInteractor->SetInteractorStyle(demoStyle);
    }

    demoInteractor->Initialize();
    
    // 注册窗口关闭事件回调（仅在网格类型时）
    if (type == VisualizationType::EMesh)
    {
        vtkSmartPointer<vtkCallbackCommand> closeCallback = vtkSmartPointer<vtkCallbackCommand>::New();
        closeCallback->SetCallback(OnWindowClose);
        demoWindow->AddObserver(vtkCommand::ExitEvent, closeCallback);
    }
    
    // 先渲染一次，确保窗口已创建并显示（在主线程中完成，确保OpenGL上下文正确初始化）
    demoWindow->Render();
    
    // 保存渲染会话状态（供Socket服务器使用）
    // 注意：此时窗口已渲染，可以安全地获取窗口句柄并启动服务器
    SetRenderSession(demoWindow, demoRender, actorCollection, demoInteractor, edgeActorCollection);
    
    // 在主线程中启动交互循环（阻塞调用，直到窗口关闭）
    // 这样确保OpenGL上下文和窗口消息循环都在主线程中正确运行
    demoInteractor->Start();
}

void RenderProcessor::show(const PREPRO_BASE_NAMESPACE::PFGroup& pfGroup, VisualizationType type)
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

    // 创建actorCollection用于Socket服务器
    vtkSmartPointer<vtkActorCollection> actorCollection = vtkSmartPointer<vtkActorCollection>::New();
    if (groupActor)
    {
        actorCollection->AddItem(groupActor);
    }

    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(500, 500);

    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    demoInteractor->SetInteractorStyle(demoStyle);

    demoInteractor->Initialize();
    
    // 注册窗口关闭事件回调（仅在网格类型时）
    if (type == VisualizationType::EMesh)
    {
        vtkSmartPointer<vtkCallbackCommand> closeCallback = vtkSmartPointer<vtkCallbackCommand>::New();
        closeCallback->SetCallback(OnWindowClose);
        demoWindow->AddObserver(vtkCommand::ExitEvent, closeCallback);
    }
    
    // 先渲染一次，确保窗口已创建并显示（在主线程中完成，确保OpenGL上下文正确初始化）
    demoWindow->Render();
    
    // 保存渲染会话状态（供Socket服务器使用）
    // 注意：此时窗口已渲染，可以安全地获取窗口句柄并启动服务器
    SetRenderSession(demoWindow, demoRender, actorCollection, demoInteractor);
    
    // 在主线程中启动交互循环（阻塞调用，直到窗口关闭）
    // 这样确保OpenGL上下文和窗口消息循环都在主线程中正确运行
    demoInteractor->Start();
}

#ifdef _WIN32
HWND RenderProcessor::showAndGetWindowHandle(const PREPRO_BASE_NAMESPACE::PFData& shownData, 
                                             VisualizationType type, 
                                             HWND parentWindowHandle,
                                             std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupList)
{
    if (0 == shownData.getGroupSize() && 0 == shownData.getVolumeSize())
    {
        return nullptr;
    }

    VTK_MODULE_INIT(vtkRenderingOpenGL2);
    VTK_MODULE_INIT(vtkInteractionStyle);
    VTK_MODULE_INIT(vtkRenderingFreeType);
    VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2);

    vtkSmartPointer<vtkRenderer> demoRender = vtkSmartPointer<vtkRenderer>::New();
    demoRender->SetBackground(0.1, 0.2, 0.4);

    // 收集所有actors，用于网格可视化控制
    vtkSmartPointer<vtkActorCollection> actorCollection = vtkSmartPointer<vtkActorCollection>::New();
    vtkSmartPointer<vtkActorCollection> edgeActorCollection = vtkSmartPointer<vtkActorCollection>::New();

    auto addActorWithEdgeOverlay = [&](vtkActor* groupActor) {
        if (!groupActor) return;
        demoRender->AddActor(groupActor);
        actorCollection->AddItem(groupActor);
        if (type == VisualizationType::EMesh)
        {
            vtkSmartPointer<vtkActor> edgeActor = createEdgeOverlayActor(groupActor);
            if (edgeActor)
            {
                demoRender->AddActor(edgeActor);
                edgeActorCollection->AddItem(edgeActor);
            }
        }
    };

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
        addActorWithEdgeOverlay(groupActor);
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
            addActorWithEdgeOverlay(groupActor);
        }
    }

    for (unsigned int i = 0; i < groupList.size(); ++i)
    {
        currentGroup = groupList[i];
        checkName = currentGroup->getName();

        vtkSmartPointer<vtkActor> groupActor = buildVTKGroup(currentGroup, type);
        addActorWithEdgeOverlay(groupActor);
    }

    vtkSmartPointer<vtkRenderWindow> demoWindow = vtkSmartPointer<vtkRenderWindow>::New();
    demoWindow->AddRenderer(demoRender);
    demoWindow->SetSize(800, 600);
    
    // 设置窗口标题
    if (type == VisualizationType::EMesh)
    {
        demoWindow->SetWindowName("网格可视化 - 按Alt+H查看帮助");
    }
    else if (type == VisualizationType::EGeomerty)
    {
        demoWindow->SetWindowName("几何可视化");
    }
    else
    {
        demoWindow->SetWindowName("可视化");
    }

    // 如果提供了父窗口句柄，将窗口嵌入到父窗口
    if (parentWindowHandle != nullptr)
    {
        // 设置父窗口，VTK会自动处理嵌入
        demoWindow->SetParentId(reinterpret_cast<void*>(parentWindowHandle));
    }

    // 创建并设置交互器
    vtkSmartPointer<vtkRenderWindowInteractor> demoInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    demoInteractor->SetRenderWindow(demoWindow);

    // 如果是网格类型，使用自定义交互样式
    if (type == VisualizationType::EMesh)
    {
        vtkSmartPointer<MeshInteractorStyle> meshStyle = vtkSmartPointer<MeshInteractorStyle>::New();
        meshStyle->SetRenderer(demoRender);
        meshStyle->SetActors(actorCollection);
        meshStyle->SetEdgeActors(edgeActorCollection);
        demoInteractor->SetInteractorStyle(meshStyle);
    }
    else
    {
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> demoStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        demoInteractor->SetInteractorStyle(demoStyle);
    }

    // 初始化窗口（但不启动交互循环，让外部程序管理）
    demoInteractor->Initialize();
    
    // 渲染一次以确保窗口已创建
    demoWindow->Render();

    // 保存渲染会话（供 Socket 服务器使用，支持 E 键边界切换等）
    SetRenderSession(demoWindow, demoRender, actorCollection, demoInteractor, edgeActorCollection);

    // 获取窗口句柄
    HWND windowHandle = reinterpret_cast<HWND>(demoWindow->GetGenericWindowId());
    
    // 如果提供了父窗口句柄，使用Windows API嵌入
    if (parentWindowHandle != nullptr && windowHandle != nullptr)
    {
        ::SetParent(windowHandle, parentWindowHandle);
        // 设置窗口样式为子窗口
        LONG_PTR style = ::GetWindowLongPtr(windowHandle, GWL_STYLE);
        style &= ~WS_POPUP;
        style |= WS_CHILD;
        ::SetWindowLongPtr(windowHandle, GWL_STYLE, style);
        // 调整窗口大小以适应父窗口
        RECT parentRect;
        if (::GetClientRect(parentWindowHandle, &parentRect))
        {
            ::SetWindowPos(windowHandle, nullptr, 0, 0, 
                          parentRect.right - parentRect.left,
                          parentRect.bottom - parentRect.top,
                          SWP_NOZORDER | SWP_SHOWWINDOW);
        }
    }

    return windowHandle;
}
#else
void* RenderProcessor::showAndGetWindowHandle(const PREPRO_BASE_NAMESPACE::PFData& shownData, 
                                               VisualizationType type, 
                                               void* parentWindowHandle,
                                               std::vector<PREPRO_BASE_NAMESPACE::PFGroup*> groupList)
{
    // 非Windows平台暂不支持嵌入模式，返回nullptr
    // 可以调用普通的show方法
    show(shownData, type, groupList);
    return nullptr;
}
#endif

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
void RenderProcessor::showResult(const PREPRO_BASE_NAMESPACE::PFData& shownData, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues& variableValues)
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

    // 创建actorCollection用于Socket服务器
    vtkSmartPointer<vtkActorCollection> actorCollection = vtkSmartPointer<vtkActorCollection>::New();
    if (demoActor)
    {
        actorCollection->AddItem(demoActor);
    }

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
    
    // 先渲染一次，确保窗口已创建并显示
    demoWindow->Render();
    
    // 保存渲染会话状态（供Socket服务器使用）
    // 注意：此时窗口已渲染，可以安全地获取窗口句柄并启动服务器
    SetRenderSession(demoWindow, demoRender, actorCollection, demoInteractor);
    
    // 在主线程中启动交互循环（阻塞调用，直到窗口关闭）
    demoInteractor->Start();
}

void RenderProcessor::showLeakPath(const PREPRO_MESH_NAMESPACE::PFLeakPathContainer& leakPath)
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

void RenderProcessor::show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType type, std::set<std::string> groupNames)
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

    // 创建actorCollection用于Socket服务器
    vtkSmartPointer<vtkActorCollection> actorCollection = vtkSmartPointer<vtkActorCollection>::New();

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
        if (groupActor)
        {
            demoRender->AddActor(groupActor);
            actorCollection->AddItem(groupActor);
        }
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
            if (groupActor)
            {
                demoRender->AddActor(groupActor);
                actorCollection->AddItem(groupActor);
            }
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
    
    // 注册窗口关闭事件回调（仅在网格类型时）
    if (type == VisualizationType::EMesh)
    {
        vtkSmartPointer<vtkCallbackCommand> closeCallback = vtkSmartPointer<vtkCallbackCommand>::New();
        closeCallback->SetCallback(OnWindowClose);
        demoWindow->AddObserver(vtkCommand::ExitEvent, closeCallback);
    }
    
    // 先渲染一次，确保窗口已创建并显示（在主线程中完成，确保OpenGL上下文正确初始化）
    demoWindow->Render();
    
    // 保存渲染会话状态（供Socket服务器使用）
    // 注意：此时窗口已渲染，可以安全地获取窗口句柄并启动服务器
    SetRenderSession(demoWindow, demoRender, actorCollection, demoInteractor);
    
    // 在主线程中启动交互循环（阻塞调用，直到窗口关闭）
    // 这样确保OpenGL上下文和窗口消息循环都在主线程中正确运行
    demoInteractor->Start();
}