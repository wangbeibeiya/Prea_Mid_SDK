#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include "json.hpp"
#include <string>
#include <vector>

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

using json = nlohmann::json;

class SocketClient {
public:
    SocketClient();
    ~SocketClient();

    bool connect(const std::string& host, int port);
    void disconnect();
    bool isConnected() const;
    bool sendCommand(const std::string& command, const json& params, json& response);

private:
    SOCKET m_socket;
#ifdef _WIN32
    WSADATA m_wsaData;
#endif
};

#endif // SOCKETCLIENT_H
