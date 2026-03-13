#include "SocketClient.h"
#include <cstring>

SocketClient::SocketClient() : m_socket(INVALID_SOCKET) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &m_wsaData);
#endif
}

SocketClient::~SocketClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool SocketClient::connect(const std::string& host, int port) {
    if (m_socket != INVALID_SOCKET) disconnect();
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) return false;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<unsigned short>(port));
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (::connect(m_socket, (sockaddr*)&addr, sizeof(addr)) != 0) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = INVALID_SOCKET;
        return false;
    }
    return true;
}

void SocketClient::disconnect() {
    if (m_socket != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = INVALID_SOCKET;
    }
}

bool SocketClient::isConnected() const {
    return m_socket != INVALID_SOCKET;
}

bool SocketClient::sendCommand(const std::string& command, const json& params, json& response) {
    if (m_socket == INVALID_SOCKET) return false;
    json req;
    req["command"] = command;
    req["params"] = params;
    std::string reqStr = req.dump();
    int len = static_cast<int>(reqStr.length());

    if (send(m_socket, (char*)&len, sizeof(len), 0) != sizeof(len)) return false;
    if (send(m_socket, reqStr.c_str(), len, 0) != len) return false;

    int respLen = 0;
    if (recv(m_socket, (char*)&respLen, sizeof(respLen), 0) != sizeof(respLen)) return false;
    if (respLen <= 0 || respLen > 65536) return false;

    std::vector<char> buf(respLen);
    int total = 0;
    while (total < respLen) {
        int n = recv(m_socket, buf.data() + total, respLen - total, 0);
        if (n <= 0) return false;
        total += n;
    }
    buf.push_back('\0');
    response = json::parse(buf.data());
    return true;
}
