#include "SocketCommandAPI.h"
#include "MeshVisualizationServer.h"
#include "VolumeVisualizationServer.h"
#include "ModelProcessingServer.h"
#include "DataInteractionServer.h"
#include "../include/Logger.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <windows.h>
#ifdef MAPPING_GEOMETRY_HAS_QT5_CORE
#include <QCoreApplication>
#endif

extern "C" {
    void SetServerInstance(MeshVisualizationServer*);
    MeshVisualizationServer* GetServerInstance();
}

static std::string getAppLogPath() {
    const char* appData = std::getenv("APPDATA");
    if (!appData || !appData[0]) return "";
    std::filesystem::path p(appData);
    p /= "PERA SIM Therm";
    if (!std::filesystem::exists(p)) {
        std::error_code ec;
        if (!std::filesystem::create_directories(p, ec)) return "";
    }
    p /= "MappingGeometry.Log";
    return p.string();
}

int main(int argc, char* argv[]) {
#ifdef MAPPING_GEOMETRY_HAS_QT5_CORE
    // SDK 内部使用 Qt，openDocument 等会访问 QCoreApplication::instance()
    // 未初始化时会导致 0xC0000005 访问冲突（Qt5Core.dll）
    QCoreApplication qtApp(argc, argv);
#endif

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "=== Pera CFD SDK 模型处理程序 ===" << std::endl;

    // 程序启动时创建日志（%APPDATA%/PERA SIM Therm/MappingGeometry.Log）
    static Logger appLogger;
    std::string logPath = getAppLogPath();
    if (!logPath.empty() && appLogger.initializeLogFile(logPath, false)) {
        appLogger.logOutput("========================================");
        appLogger.logOutput("程序启动 - " + appLogger.getCurrentTimeString());
        appLogger.logOutput("日志路径: " + logPath);
        appLogger.logOutput("========================================\n");
    }

    int socketPort = 12345;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-socket") {
            continue;
        } else if (arg == "-port" && i + 1 < argc) {
            try {
                socketPort = std::stoi(argv[++i]);
                if (socketPort < 1 || socketPort > 65535) {
                    std::cerr << "错误: 端口号必须在 1-65535 之间" << std::endl;
                    return 1;
                }
            } catch (...) {
                std::cerr << "错误: -port 需要有效的端口号" << std::endl;
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "\n用法: MappingGeometry.exe [选项]\n" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  -socket -port <端口>  指定Socket监听端口（默认54321）" << std::endl;
            std::cout << "  --help, -h            显示此帮助信息\n" << std::endl;
            std::cout << "示例:" << std::endl;
            std::cout << "  MappingGeometry.exe -socket -port 54321" << std::endl;
            std::cout << "\n启动后通过 Socket 发送命令执行几何匹配、网格划分等完整流程。\n" << std::endl;
            return 0;
        }
    }

    std::cout << "\n正在启动 Socket 服务器，端口: " << socketPort << "..." << std::endl;

    static SocketCommandAPI sharedSocket(socketPort);
    static MeshVisualizationServer meshServer(&sharedSocket);
    static VolumeVisualizationServer volumeServer(&sharedSocket);
    static ModelProcessingServer modelProcessingServer(&sharedSocket, &appLogger);
    static DataInteractionServer dataInteractionServer(&sharedSocket, &modelProcessingServer);

    // 内置诊断命令：列出已注册命令
    {
        SocketCommandAPI* api = &sharedSocket;
        sharedSocket.registerCommand("ListCommands", [api](const nlohmann::json&) {
            nlohmann::json r;
            r["success"] = true;
            r["commands"] = api->getRegisteredCommands();
            return r;
        });
    }

    std::cout << "已注册命令: ";
    for (const auto& c : sharedSocket.getRegisteredCommands()) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    if (!sharedSocket.start()) {
        std::cerr << "Socket 服务器启动失败" << std::endl;
        return 1;
    }

    SetServerInstance(&meshServer);
    std::cout << "Socket 服务器已启动，端口: " << socketPort << std::endl;
    std::cout << "通过 Socket 发送命令执行完整流程，按 Enter 退出..." << std::endl;

    std::cin.get();
    return 0;
}
