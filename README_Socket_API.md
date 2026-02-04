# 网格可视化 Socket API 调用说明

## 概述

网格可视化服务器通过Socket接收JSON格式的命令，控制网格可视化的各项操作。服务器在渲染窗口成功显示后自动启动，默认监听端口 **54321**。

## 通信协议

### 连接方式
- **协议**: TCP/IP
- **端口**: 54321（默认）
- **数据格式**: JSON
- **编码**: UTF-8

### 数据包格式

每个数据包由两部分组成：
1. **长度字段**（4字节，小端序）：JSON数据的字节长度
2. **JSON数据**：UTF-8编码的JSON字符串

### 请求格式

```json
{
  "command": "命令名称",
  "params": {
    "参数名1": "参数值1",
    "参数名2": "参数值2"
  }
}
```

### 响应格式

```json
{
  "success": true/false,
  "字段名": "字段值",
  "error": "错误信息（仅在success为false时存在）"
}
```

---

## 命令列表

### 1. GetWindowHandle - 获取窗口句柄

获取当前渲染窗口的句柄，用于将窗口嵌入到其他应用程序中。

**请求示例**:
```json
{
  "command": "GetWindowHandle",
  "params": {}
}
```

**成功响应**:
```json
{
  "success": true,
  "windowHandle": 12345678
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染窗口未初始化"
}
```

**说明**:
- `windowHandle`: Windows平台为HWND句柄的数值表示（uintptr_t），其他平台为窗口ID
- 如果窗口未初始化，返回错误

---

### 2. SetTransparency - 设置透明度

设置网格的透明度（不透明度）。

**请求示例**:
```json
{
  "command": "SetTransparency",
  "params": {
    "opacity": 0.5
  }
}
```

**参数说明**:
- `opacity` (double, 可选): 不透明度值，范围 0.0-1.0
  - 0.0: 完全透明
  - 1.0: 完全不透明（默认）
  - 如果未提供或超出范围，会自动限制在有效范围内

**成功响应**:
```json
{
  "success": true,
  "opacity": 0.5
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 3. ToggleEdges - 切换边界显示

开启或关闭网格边界的显示。

**请求示例**:
```json
{
  "command": "ToggleEdges",
  "params": {
    "enable": true
  }
}
```

**参数说明**:
- `enable` (boolean, 可选): 是否显示边界
  - `true`: 显示边界（默认）
  - `false`: 隐藏边界

**成功响应**:
```json
{
  "success": true,
  "edgesEnabled": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 4. SetRepresentation - 设置显示模式

设置网格的显示模式（实体、线框或点）。

**请求示例**:
```json
{
  "command": "SetRepresentation",
  "params": {
    "representation": "wireframe"
  }
}
```

**参数说明**:
- `representation` (string, 可选): 显示模式
  - `"surface"`: 实体模式（默认）
  - `"wireframe"`: 线框模式
  - `"points"`: 点模式

**成功响应**:
```json
{
  "success": true,
  "representation": "wireframe"
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 5. ToggleWireframe - 切换线框模式

开启或关闭线框显示模式。

**请求示例**:
```json
{
  "command": "ToggleWireframe",
  "params": {
    "enable": true
  }
}
```

**参数说明**:
- `enable` (boolean, 可选): 是否启用线框模式
  - `true`: 启用线框模式
  - `false`: 切换到实体模式（默认）

**成功响应**:
```json
{
  "success": true,
  "wireframeEnabled": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 6. TogglePoints - 切换点模式

开启或关闭点显示模式。

**请求示例**:
```json
{
  "command": "TogglePoints",
  "params": {
    "enable": true
  }
}
```

**参数说明**:
- `enable` (boolean, 可选): 是否启用点模式
  - `true`: 启用点模式（点大小为5）
  - `false`: 切换到实体模式（默认）

**成功响应**:
```json
{
  "success": true,
  "pointsEnabled": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 7. ToggleColorByGroup - 切换按组着色

开启或关闭按组着色功能。启用后，不同的组会使用不同的颜色显示。

**请求示例**:
```json
{
  "command": "ToggleColorByGroup",
  "params": {
    "enable": true
  }
}
```

**参数说明**:
- `enable` (boolean, 可选): 是否启用按组着色
  - `true`: 启用按组着色，使用8种颜色循环（红、绿、蓝、黄、洋红、青、橙、紫）
  - `false`: 恢复默认灰色（默认）

**成功响应**:
```json
{
  "success": true,
  "colorByGroupEnabled": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染actors未初始化"
}
```

---

### 8. ResetCamera - 重置相机视图

重置相机到默认视图，自动调整视角以显示所有对象。

**请求示例**:
```json
{
  "command": "ResetCamera",
  "params": {}
}
```

**成功响应**:
```json
{
  "success": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染器未初始化"
}
```

---

### 9. Render - 强制渲染

强制刷新渲染窗口，立即更新显示。

**请求示例**:
```json
{
  "command": "Render",
  "params": {}
}
```

**成功响应**:
```json
{
  "success": true
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "渲染窗口未初始化"
}
```

---

### 10. SetSize - 设置窗口大小

设置渲染窗口的宽度和高度。

**请求示例**:
```json
{
  "command": "SetSize",
  "params": {
    "width": 1024,
    "height": 768
  }
}
```

**参数说明**:
- `width` (integer, 可选): 窗口宽度（像素），默认800
- `height` (integer, 可选): 窗口高度（像素），默认600
- 必须大于0

**成功响应**:
```json
{
  "success": true,
  "width": 1024,
  "height": 768
}
```

**失败响应**:
```json
{
  "success": false,
  "error": "无效的窗口尺寸"
}
```

或

```json
{
  "success": false,
  "error": "渲染窗口未初始化"
}
```

---

## 客户端示例代码

### Python 示例

```python
import socket
import json
import struct

class MeshVisualizationClient:
    def __init__(self, host='localhost', port=54321):
        self.host = host
        self.port = port
        self.socket = None
    
    def connect(self):
        """连接到服务器"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        print(f"已连接到服务器 {self.host}:{self.port}")
    
    def disconnect(self):
        """断开连接"""
        if self.socket:
            self.socket.close()
            self.socket = None
    
    def send_command(self, command, params=None):
        """发送命令并接收响应"""
        if not self.socket:
            raise Exception("未连接到服务器")
        
        # 构建请求
        request = {
            "command": command,
            "params": params or {}
        }
        
        # 转换为JSON字符串
        json_str = json.dumps(request, ensure_ascii=False)
        json_bytes = json_str.encode('utf-8')
        
        # 发送长度（4字节，小端序）
        length = len(json_bytes)
        self.socket.send(struct.pack('<I', length))
        
        # 发送JSON数据
        self.socket.send(json_bytes)
        
        # 接收响应长度
        length_bytes = self.socket.recv(4)
        if len(length_bytes) != 4:
            raise Exception("接收响应长度失败")
        response_length = struct.unpack('<I', length_bytes)[0]
        
        # 接收响应数据
        response_bytes = b''
        while len(response_bytes) < response_length:
            chunk = self.socket.recv(response_length - len(response_bytes))
            if not chunk:
                raise Exception("接收响应数据失败")
            response_bytes += chunk
        
        # 解析JSON响应
        response_str = response_bytes.decode('utf-8')
        return json.loads(response_str)
    
    def get_window_handle(self):
        """获取窗口句柄"""
        return self.send_command("GetWindowHandle")
    
    def set_transparency(self, opacity):
        """设置透明度"""
        return self.send_command("SetTransparency", {"opacity": opacity})
    
    def toggle_edges(self, enable=True):
        """切换边界显示"""
        return self.send_command("ToggleEdges", {"enable": enable})
    
    def set_representation(self, representation):
        """设置显示模式"""
        return self.send_command("SetRepresentation", {"representation": representation})
    
    def toggle_wireframe(self, enable=True):
        """切换线框模式"""
        return self.send_command("ToggleWireframe", {"enable": enable})
    
    def toggle_points(self, enable=True):
        """切换点模式"""
        return self.send_command("TogglePoints", {"enable": enable})
    
    def toggle_color_by_group(self, enable=True):
        """切换按组着色"""
        return self.send_command("ToggleColorByGroup", {"enable": enable})
    
    def reset_camera(self):
        """重置相机"""
        return self.send_command("ResetCamera")
    
    def render(self):
        """强制渲染"""
        return self.send_command("Render")
    
    def set_size(self, width, height):
        """设置窗口大小"""
        return self.send_command("SetSize", {"width": width, "height": height})


# 使用示例
if __name__ == "__main__":
    client = MeshVisualizationClient()
    try:
        client.connect()
        
        # 获取窗口句柄
        response = client.get_window_handle()
        if response["success"]:
            print(f"窗口句柄: {response['windowHandle']}")
        
        # 设置透明度为50%
        response = client.set_transparency(0.5)
        print(f"设置透明度: {response}")
        
        # 切换到线框模式
        response = client.set_representation("wireframe")
        print(f"设置显示模式: {response}")
        
        # 启用按组着色
        response = client.toggle_color_by_group(True)
        print(f"按组着色: {response}")
        
        # 重置相机
        response = client.reset_camera()
        print(f"重置相机: {response}")
        
    except Exception as e:
        print(f"错误: {e}")
    finally:
        client.disconnect()
```

### C++ 示例

```cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include "json.hpp"  // 使用nlohmann/json

using json = nlohmann::json;

class MeshVisualizationClient {
private:
    SOCKET m_socket;
    std::string m_host;
    int m_port;
    
    bool sendData(const char* data, int length) {
        int sent = 0;
        while (sent < length) {
            int result = send(m_socket, data + sent, length - sent, 0);
            if (result == SOCKET_ERROR) return false;
            sent += result;
        }
        return true;
    }
    
    bool receiveData(char* buffer, int length) {
        int received = 0;
        while (received < length) {
            int result = recv(m_socket, buffer + received, length - received, 0);
            if (result <= 0) return false;
            received += result;
        }
        return true;
    }
    
public:
    MeshVisualizationClient(const std::string& host = "localhost", int port = 54321)
        : m_socket(INVALID_SOCKET), m_host(host), m_port(port) {}
    
    bool connect() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }
        
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_port);
        inet_pton(AF_INET, m_host.c_str(), &serverAddr.sin_addr);
        
        if (::connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(m_socket);
            WSACleanup();
            return false;
        }
        
        return true;
    }
    
    void disconnect() {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
            WSACleanup();
            m_socket = INVALID_SOCKET;
        }
    }
    
    json sendCommand(const std::string& command, const json& params = json::object()) {
        json request;
        request["command"] = command;
        request["params"] = params;
        
        std::string jsonStr = request.dump();
        int length = static_cast<int>(jsonStr.length());
        
        // 发送长度（小端序）
        if (!sendData((char*)&length, sizeof(length))) {
            return json{{"success", false}, {"error", "发送失败"}};
        }
        
        // 发送JSON数据
        if (!sendData(jsonStr.c_str(), length)) {
            return json{{"success", false}, {"error", "发送失败"}};
        }
        
        // 接收响应长度
        int responseLength = 0;
        if (!receiveData((char*)&responseLength, sizeof(responseLength))) {
            return json{{"success", false}, {"error", "接收失败"}};
        }
        
        // 接收响应数据
        std::vector<char> buffer(responseLength);
        if (!receiveData(buffer.data(), responseLength)) {
            return json{{"success", false}, {"error", "接收失败"}};
        }
        
        std::string responseStr(buffer.data(), responseLength);
        return json::parse(responseStr);
    }
};

// 使用示例
int main() {
    MeshVisualizationClient client;
    
    if (!client.connect()) {
        std::cerr << "连接失败" << std::endl;
        return 1;
    }
    
    // 获取窗口句柄
    json response = client.sendCommand("GetWindowHandle");
    if (response["success"]) {
        std::cout << "窗口句柄: " << response["windowHandle"] << std::endl;
    }
    
    // 设置透明度
    response = client.sendCommand("SetTransparency", {{"opacity", 0.5}});
    std::cout << "设置透明度: " << response.dump() << std::endl;
    
    // 切换到线框模式
    response = client.sendCommand("SetRepresentation", {{"representation", "wireframe"}});
    std::cout << "设置显示模式: " << response.dump() << std::endl;
    
    client.disconnect();
    return 0;
}
```

---

## 错误处理

所有命令都可能返回以下通用错误：

1. **JSON解析错误**: 请求格式不正确
   ```json
   {
     "success": false,
     "error": "JSON解析错误: ..."
   }
   ```

2. **未知命令**: 命令名称不存在
   ```json
   {
     "success": false,
     "error": "未知命令: xxx"
   }
   ```

3. **缺少command字段**: 请求格式不完整
   ```json
   {
     "success": false,
     "error": "缺少command字段或格式错误"
   }
   ```

4. **渲染未初始化**: 渲染窗口或actors未初始化
   ```json
   {
     "success": false,
     "error": "渲染窗口未初始化"
   }
   ```
   或
   ```json
   {
     "success": false,
     "error": "渲染actors未初始化"
   }
   ```

---

## 注意事项

1. **服务器启动时机**: 服务器在渲染窗口成功显示后自动启动，在此之前连接会失败
2. **阻塞操作**: `RenderProcessor::show` 是阻塞调用，会启动交互循环，直到用户关闭窗口
3. **线程安全**: 服务器支持多客户端连接，每个客户端在独立线程中处理
4. **窗口句柄**: Windows平台返回的窗口句柄可以直接用于 `SetParent` 等Windows API
5. **参数验证**: 所有参数都有默认值，如果未提供或无效，会使用默认值或自动修正
6. **实时更新**: 大部分命令执行后会立即触发渲染，更新显示

---

## 版本信息

- **API版本**: 1.0
- **最后更新**: 2025-01-28
- **默认端口**: 54321
