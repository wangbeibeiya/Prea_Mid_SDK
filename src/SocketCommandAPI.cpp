#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "SocketCommandAPI.h"
#include <iostream>
#include <cstring>

SocketCommandAPI::SocketCommandAPI(int port)
    : m_port(port)
    , m_running(false)
#ifdef _WIN32
    , m_listenSocket(INVALID_SOCKET)
#else
    , m_listenSocket(-1)
#endif
{
}

SocketCommandAPI::~SocketCommandAPI()
{
    stop();
}

void SocketCommandAPI::registerCommand(const std::string& command, CommandHandler handler)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_commandHandlers[command] = std::move(handler);
}

void SocketCommandAPI::unregisterCommand(const std::string& command)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_commandHandlers.erase(command);
}

std::vector<std::string> SocketCommandAPI::getRegisteredCommands() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> result;
    for (const auto& p : m_commandHandlers)
    {
        result.push_back(p.first);
    }
    return result;
}

bool SocketCommandAPI::start()
{
    if (m_running)
    {
        return true;
    }

#ifdef _WIN32
    int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (result != 0)
    {
        std::cerr << "[SocketCommandAPI] WSAStartup失败: " << result << std::endl;
        return false;
    }
#endif

#ifdef _WIN32
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
#else
    m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenSocket < 0)
#endif
    {
        std::cerr << "[SocketCommandAPI] 创建socket失败" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    int opt = 1;
#ifdef _WIN32
    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
#else
    if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
#endif
    {
        std::cerr << "[SocketCommandAPI] 设置socket选项失败" << std::endl;
#ifdef _WIN32
        closesocket(m_listenSocket);
        WSACleanup();
#else
        close(m_listenSocket);
#endif
        return false;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<unsigned short>(m_port));

#ifdef _WIN32
    if (bind(m_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "[SocketCommandAPI] 绑定地址失败，端口: " << m_port << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }
    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[SocketCommandAPI] 监听失败" << std::endl;
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }
#else
    if (bind(m_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "[SocketCommandAPI] 绑定地址失败，端口: " << m_port << std::endl;
        close(m_listenSocket);
        return false;
    }
    if (listen(m_listenSocket, 5) < 0)
    {
        std::cerr << "[SocketCommandAPI] 监听失败" << std::endl;
        close(m_listenSocket);
        return false;
    }
#endif

    m_running = true;
    m_serverThread = std::thread(&SocketCommandAPI::serverLoop, this);

    std::cout << "[SocketCommandAPI] 服务器已启动，监听端口: " << m_port << std::endl;
    return true;
}

void SocketCommandAPI::stop()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;

#ifdef _WIN32
    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
#else
    if (m_listenSocket >= 0)
    {
        close(m_listenSocket);
        m_listenSocket = -1;
    }
#endif

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    std::cout << "[SocketCommandAPI] 服务器已停止" << std::endl;
}

void SocketCommandAPI::serverLoop()
{
    while (m_running)
    {
#ifdef _WIN32
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
#else
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket < 0)
#endif
        {
            if (m_running)
            {
                std::cerr << "[SocketCommandAPI] 接受连接失败" << std::endl;
            }
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "[SocketCommandAPI] 客户端连接: " << clientIP << ":"
                  << ntohs(clientAddr.sin_port) << std::endl;

        std::thread clientThread(&SocketCommandAPI::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void SocketCommandAPI::handleClient(SOCKET clientSocket)
{
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    while (m_running)
    {
        int length = 0;
        int received = receiveData(clientSocket, (char*)&length, sizeof(length));
        if (received != sizeof(length) || length <= 0 || length > BUFFER_SIZE)
        {
            break;
        }

        received = receiveData(clientSocket, buffer, length);
        if (received != length)
        {
            break;
        }

        buffer[length] = '\0';

        try
        {
            json commandJson = json::parse(buffer);
            json response = processCommand(commandJson);
            if (!sendResponse(clientSocket, response))
            {
                break;
            }
        }
        catch (const json::exception& e)
        {
            json errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = std::string("JSON解析错误: ") + e.what();
            sendResponse(clientSocket, errorResponse);
        }
        catch (const std::exception& e)
        {
            json errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = std::string("处理命令时发生错误: ") + e.what();
            sendResponse(clientSocket, errorResponse);
        }
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif

    std::cout << "[SocketCommandAPI] 客户端断开连接" << std::endl;
}

json SocketCommandAPI::processCommand(const json& commandJson)
{
    json response;

    if (!commandJson.contains("command") || !commandJson["command"].is_string())
    {
        response["success"] = false;
        response["error"] = "缺少command字段或格式错误";
        return response;
    }

    std::string command = commandJson["command"].get<std::string>();
    json params = json::object();
    if (commandJson.contains("params") && commandJson["params"].is_object())
        params = commandJson["params"];

    CommandHandler handler;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_commandHandlers.find(command);
        if (it != m_commandHandlers.end())
        {
            handler = it->second;
        }
    }

    if (handler)
    {
        try
        {
            return handler(params);
        }
        catch (const std::exception& e)
        {
            response["success"] = false;
            response["error"] = std::string("命令执行异常: ") + e.what();
            return response;
        }
    }
    else
    {
        response["success"] = false;
        response["error"] = "未知命令: " + command;
        return response;
    }
}

bool SocketCommandAPI::sendResponse(SOCKET clientSocket, const json& response)
{
    std::string responseStr = response.dump();
    int length = static_cast<int>(responseStr.length());

#ifdef _WIN32
    int sent = send(clientSocket, (char*)&length, sizeof(length), 0);
    if (sent != sizeof(length))
    {
        return false;
    }
    sent = send(clientSocket, responseStr.c_str(), length, 0);
    return sent == length;
#else
    int sent = send(clientSocket, &length, sizeof(length), 0);
    if (sent != sizeof(length))
    {
        return false;
    }
    sent = send(clientSocket, responseStr.c_str(), length, 0);
    return sent == length;
#endif
}

int SocketCommandAPI::receiveData(SOCKET clientSocket, char* buffer, int bufferSize)
{
    int totalReceived = 0;

    while (totalReceived < bufferSize)
    {
#ifdef _WIN32
        int received = recv(clientSocket, buffer + totalReceived, bufferSize - totalReceived, 0);
        if (received <= 0)
        {
            return -1;
        }
#else
        int received = recv(clientSocket, buffer + totalReceived, bufferSize - totalReceived, 0);
        if (received <= 0)
        {
            return -1;
        }
#endif
        totalReceived += received;
    }

    return totalReceived;
}
