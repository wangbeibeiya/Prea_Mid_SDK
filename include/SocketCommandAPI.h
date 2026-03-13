#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>

#include "json.hpp"
using json = nlohmann::json;

/**
 * @brief 通用 Socket 命令 API 类
 *
 * 负责 Socket 通信与命令分发，支持类似 API 的命令扩展。
 * 外部软件可通过 Socket 实时发送 JSON 命令，实现交互。
 *
 * 使用方式：
 *   1. 创建实例并指定端口
 *   2. 通过 registerCommand() 注册命令处理函数
 *   3. 调用 start() 启动服务
 *
 * 示例：
 *   auto api = std::make_unique<SocketCommandAPI>(54321);
 *   api->registerCommand("MyCommand", [](const json& params) { ... });
 *   api->start();
 */
class SocketCommandAPI
{
public:
    using CommandHandler = std::function<json(const json&)>;

    explicit SocketCommandAPI(int port = 54321);
    ~SocketCommandAPI();

    void registerCommand(const std::string& command, CommandHandler handler);
    void unregisterCommand(const std::string& command);
    std::vector<std::string> getRegisteredCommands() const;

    bool start();
    void stop();
    bool isRunning() const { return m_running; }
    int getPort() const { return m_port; }

private:
    int m_port;
    std::atomic<bool> m_running;
    std::thread m_serverThread;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, CommandHandler> m_commandHandlers;

    SOCKET m_listenSocket;
#ifdef _WIN32
    WSADATA m_wsaData;
#endif

    void serverLoop();
    void handleClient(SOCKET clientSocket);
    json processCommand(const json& commandJson);
    bool sendResponse(SOCKET clientSocket, const json& response);
    int receiveData(SOCKET clientSocket, char* buffer, int bufferSize);
};
