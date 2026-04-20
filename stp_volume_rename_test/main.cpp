/*
 * STP 导入 → 快速修复 → 查找体 → 打印体名 → 将第一个体重命名为 Test → getAllData → 再次打印体名
 * 用法: StpVolumeRenameTest.exe <path-to-file.stp>
 */

#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/pfGroupData.h>
#include <geometry/pfGeometry.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometryData.h>
#include <mesh/pfMeshBuilder.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(MAPPING_GEOMETRY_HAS_QT5_CORE)
#include <QCoreApplication>
#endif

namespace {

void printVolumeNames(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry, const char* title)
{
    PREPRO_BASE_NAMESPACE::PFData data;
    if (geometry->getAllData(data) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] getAllData 失败: " << title << std::endl;
        return;
    }

    int n = data.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = data.getVolumes();
    std::cout << "[StpVolumeRenameTest] " << title << " — 体数量: " << n << std::endl;
    for (int i = 0; i < n; ++i)
    {
        if (!volumes || !volumes[i])
        {
            std::cout << "  [" << i << "] (null)" << std::endl;
            continue;
        }
        char* name = volumes[i]->getName();
        std::cout << "  [" << i << "] " << (name ? name : "(无名称)") << std::endl;
    }
}

std::string firstVolumeName(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry, bool& ok)
{
    ok = false;
    PREPRO_BASE_NAMESPACE::PFData data;
    if (geometry->getAllData(data) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
        return {};
    int n = data.getVolumeSize();
    PREPRO_BASE_NAMESPACE::PFVolume** volumes = data.getVolumes();
    if (n <= 0 || !volumes || !volumes[0])
        return {};
    char* name = volumes[0]->getName();
    if (!name || !name[0])
        return {};
    ok = true;
    return std::string(name);
}

} // namespace

int main(int argc, char* argv[])
{
#if defined(MAPPING_GEOMETRY_HAS_QT5_CORE)
    QCoreApplication qtApp(argc, argv);
#endif
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 2)
    {
        std::cerr << "用法: " << (argv[0] ? argv[0] : "StpVolumeRenameTest") << " <stp文件路径>" << std::endl;
        return 1;
    }

    const std::string stpPath = argv[1];
    if (!std::filesystem::exists(stpPath))
    {
        std::cerr << "[StpVolumeRenameTest] 文件不存在: " << stpPath << std::endl;
        return 1;
    }

    PREPRO_BASE_NAMESPACE::PFApplication app;

    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geoBuilder =
        PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (!geoBuilder)
    {
        std::cerr << "[StpVolumeRenameTest] 无法获取 PFGeometryBuilder" << std::endl;
        return 1;
    }
    if (app.addEnvironment(geoBuilder) == PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense)
    {
        std::cerr << "[StpVolumeRenameTest] 无 PERA SIM 许可证" << std::endl;
        return 1;
    }

    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (meshBuilder)
        app.addEnvironment(meshBuilder);

    PREPRO_BASE_NAMESPACE::PFDocument* doc = app.newDocument();
    if (!doc)
    {
        std::cerr << "[StpVolumeRenameTest] newDocument 失败" << std::endl;
        return 1;
    }

    auto* geometry = dynamic_cast<PREPRO_GEOMETRY_NAMESPACE::PFGeometry*>(doc->getGeometryEnvironment());
    if (!geometry)
    {
        std::cerr << "[StpVolumeRenameTest] 无法获取几何环境" << std::endl;
        return 1;
    }

    geometry->setCADImportGroupMode(PREPRO_BASE_NAMESPACE::PFImportGroupMode::EFromBodyName);

    std::cout << "[StpVolumeRenameTest] 导入: " << stpPath << std::endl;
    if (geometry->importGeometry(stpPath.c_str()) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] 导入几何失败" << std::endl;
        return 1;
    }

    PREPRO_GEOMETRY_NAMESPACE::QuickRepairParameters repairParams;
    repairParams.stitchOption = PREPRO_GEOMETRY_NAMESPACE::StitchOptions::EStitchAndIntersection;
    repairParams.stitchTolerance = 1e-5;
    std::cout << "[StpVolumeRenameTest] 快速修复..." << std::endl;
    if (geometry->quickRepair(repairParams) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] 快速修复失败" << std::endl;
        return 1;
    }

    std::cout << "[StpVolumeRenameTest] 查找体..." << std::endl;
    if (geometry->findVolumes() != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] 查找体失败" << std::endl;
        return 1;
    }

    printVolumeNames(geometry, "重命名前 — 当前所有体名称");

    bool hasFirst = false;
    std::string oldName = firstVolumeName(geometry, hasFirst);
    if (!hasFirst)
    {
        std::cerr << "[StpVolumeRenameTest] 未找到可重命名的第一个体（无体或无名称）" << std::endl;
        return 1;
    }

    std::cout << "[StpVolumeRenameTest] 将第一个体重命名: \"" << oldName << "\" -> \"Test\"" << std::endl;
    if (geometry->renameVolume(oldName.c_str(), "Test") != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] renameVolume 失败" << std::endl;
        return 1;
    }

    PREPRO_BASE_NAMESPACE::PFData refreshed;
    if (geometry->getAllData(refreshed) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cerr << "[StpVolumeRenameTest] 重命名后 getAllData 失败" << std::endl;
        return 1;
    }
    (void)refreshed;

    printVolumeNames(geometry, "重命名后 — 当前所有体名称（已再次 getAllData）");

    std::cout << "[StpVolumeRenameTest] 完成" << std::endl;
    return 0;
}
