# Socket 命令 API 文档

## 概述

MappingGeometry 通过 Socket 提供 JSON 格式的命令接口，外部程序可连接并发送命令执行几何导入、网格划分、可视化等操作。

### 通信协议

- **传输**：TCP Socket
- **格式**：JSON
- **默认端口**：12345（可通过 `-port` 参数修改）

### 请求格式

```json
{
  "command": "命令名",
  "params": {
    "参数名": "参数值"
  }
}
```

### 响应格式

```json
{
  "success": true,
  "message": "提示信息",
  "其他字段": "..."
}
```

失败时返回 `"success": false` 和 `"error": "错误描述"`。

---

## 命令列表

### 1. 系统与查询

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **ListCommands** | 列出所有已注册命令 | 无 | `commands`: 命令名数组 |

---

### 2. 几何流程

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **ImportGeometryModel** | 导入几何模型 | `jsonPath`: 项目 JSON 路径（必填） | `sessionId`: 会话 ID |
| **ExecuteGeometryProcessing** | 几何处理（quickRepair + findVolumes） | `sessionId`: 会话 ID（必填）<br>`enableQuickRepair`: 是否快速修复（默认 true）<br>`enableFindVolumes`: 是否查找体（默认 true） | `message` |
| **ExecuteGeometryMatching** | 几何识别匹配（analyzeVolumes + 保存 ppcf） | `sessionId`: 会话 ID（必填）<br>`jsonPath`: 项目 JSON 路径（必填） | `message` |
| **SavePpcf** | 保存 ppcf（有啥保存啥：有几何保存几何，有网格+几何保存网格+几何） | `savePath`: 保存路径（必填） | `message`, `savePath` |
| **CloseSession** | 关闭会话 | `sessionId`: 会话 ID（必填） | `message` |
| **DeleteVolumeByName** | 按名称删除体 | `sessionId`: 会话 ID（必填）<br>`volumeName`: 体名称（必填） | `message`, `deletedVolume` |
| **GetUnmatchedVolumeNames** | 获取未匹配的体名称 | `sessionId`: 会话 ID（必填） | `unmatchedVolumeNames`, `count` |

---

### 3. 网格流程

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **ExecuteMeshGeneration** | 执行网格划分 | `jsonPath`: 项目 JSON 路径（必填）<br>`sessionId`: 可选，复用几何 session 的 GeometryAPI<br>`ppcfPath`: 可选，几何 ppcf 路径（空则从 json 推导） | `message` |
| **GetMeshQuality** | 获取网格质量 | `ppcfPath`: 包含网格的 ppcf 路径（必填） | `elementCount`, `invalidCount`, `worstQuality`, `bestQuality`, `averageQuality`, `qualityRanges` |
| **ImportPpcf** | 导入 ppcf 并刷新顶层数据 | `ppcfPath`: ppcf 文件路径（必填）<br>`importMode`: "geometry" 使用 openDocument / "mesh" 使用 importMesh（默认 geometry） | `message`, `ppcfPath`, `importMode` |

---

### 4. 数据查询

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **GetVolumeListNames** | 获取体名称列表 | `sessionId`: 会话 ID<br>或 `ppcfPath`: ppcf 路径 | `volumeNames` |
| **GetFaceGroupNamesByVolume** | 按体获取面组名称 | `sessionId`: 会话 ID<br>`ppcfPath`: ppcf 路径<br>`volumeName`: 体名称（必填） | `volumeName`, `faceGroupNames` |
| **RefreshSessionData** | 刷新会话顶层数据 | `sessionId`: 会话 ID（必填） | `message` |

---

### 5. 几何可视化

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **ShowGeometry** | 显示几何 | `sessionId`: 会话 ID（必填） | `message` |

---

### 6. 网格可视化

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **ShowMesh** | 显示网格 | `ppcfPath`: 可选，ppcf 路径；空则复用 ExecuteMeshGeneration 后的 GeometryAPI | `message`, `ppcfPath` |
| **GetMeshWindowHandle** | 获取网格渲染窗口句柄 | 无 | `windowHandle`, `windowTitle` |
| **ResetMeshCamera** | 重置网格相机 | 无 | `message` |
| **RenderMesh** | 渲染网格 | 无 | `message` |
| **SetMeshSize** | 设置网格窗口大小 | `width`, `height` | `message` |
| **ToggleMeshEdges** | 切换网格边线显示 | `enable`: true/false | `message` |
| **SetMeshRepresentation** | 设置网格显示模式 | `representation`: "surface" / "wireframe" / "points" | `message` |
| **ToggleMeshWireframe** | 切换线框模式 | `enable`: true/false | `message` |
| **ToggleMeshPoints** | 切换点显示 | `enable`: true/false | `message` |
| **SetMeshTransparency** | 设置网格透明度 | `opacity`: 0–1 | `message` |
| **ToggleMeshColorByGroup** | 切换按组着色 | `enable`: true/false | `message` |

---

### 7. 体可视化

| 命令 | 说明 | 参数 | 返回 |
|------|------|------|------|
| **GetVolumeWindowHandle** | 获取体渲染窗口句柄 | 无 | `windowHandle` |
| **ResetVolumeCamera** | 重置体相机 | 无 | `message` |
| **RenderVolume** | 渲染体 | 无 | `message` |
| **SetVolumeSize** | 设置体窗口大小 | `width`, `height` | `message` |
| **RenderVolumesWithOpacity** | 按体渲染并设置透明度 | `volumeName`: 体名称<br>`opacity`: 0–1 | `message` |

---

## 各命令使用示例

### 1. 系统与查询

**ListCommands**
```json
// 请求
{"command": "ListCommands", "params": {}}

// 响应
{"success": true, "commands": ["ListCommands", "ImportGeometryModel", "ExecuteGeometryProcessing", ...]}
```

---

### 2. 几何流程

**ImportGeometryModel**
```json
// 请求
{"command": "ImportGeometryModel", "params": {"jsonPath": "F:/Project/T1230/T1230.json"}}

// 响应
{"success": true, "message": "几何模型导入成功", "sessionId": "geom_1"}
```

**ExecuteGeometryProcessing**
```json
// 请求
{"command": "ExecuteGeometryProcessing", "params": {"sessionId": "geom_1"}}
// 或带选项
{"command": "ExecuteGeometryProcessing", "params": {"sessionId": "geom_1", "enableQuickRepair": true, "enableFindVolumes": true}}

// 响应
{"success": true, "message": "几何处理完成"}
```

**ExecuteGeometryMatching**
```json
// 请求
{"command": "ExecuteGeometryMatching", "params": {"sessionId": "geom_1", "jsonPath": "F:/Project/T1230/T1230.json"}}

// 响应
{"success": true, "message": "几何识别匹配完成"}
```

**SavePpcf**
```json
// 请求（保存路径必填，有啥保存啥：有几何保存几何，有网格+几何保存网格+几何）
{"command": "SavePpcf", "params": {"savePath": "F:/Project/T1230/T1230.ppcf"}}

// 响应
{"success": true, "message": "ppcf 已保存（有啥保存啥：有几何保存几何，有网格+几何保存网格+几何）", "savePath": "F:/Project/T1230/T1230.ppcf"}
```

**CloseSession**
```json
// 请求
{"command": "CloseSession", "params": {"sessionId": "geom_1"}}

// 响应
{"success": true, "message": "会话已关闭"}
```

**DeleteVolumeByName**
```json
// 请求
{"command": "DeleteVolumeByName", "params": {"sessionId": "geom_1", "volumeName": "unwanted_volume"}}

// 响应
{"success": true, "message": "已删除体: unwanted_volume", "sessionId": "geom_1", "deletedVolume": "unwanted_volume"}
```

**GetUnmatchedVolumeNames**
```json
// 请求
{"command": "GetUnmatchedVolumeNames", "params": {"sessionId": "geom_1"}}

// 响应
{"success": true, "sessionId": "geom_1", "unmatchedVolumeNames": ["vol_a", "vol_b"], "count": 2}
```

---

### 3. 网格流程

**ExecuteMeshGeneration**
```json
// 请求（无 session 复用，需 ppcf 已存在）
{"command": "ExecuteMeshGeneration", "params": {"jsonPath": "F:/Project/T1230/T1230.json", "ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 请求（复用几何 session）
{"command": "ExecuteMeshGeneration", "params": {"jsonPath": "F:/Project/T1230/T1230.json", "sessionId": "geom_1"}}

// 响应
{"success": true, "message": "网格划分完成"}
```

**GetMeshQuality**
```json
// 请求
{"command": "GetMeshQuality", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 响应
{
  "success": true,
  "ppcfPath": "F:/Project/T1230/T1230.ppcf",
  "elementCount": 125000,
  "invalidCount": 0,
  "worstQuality": 0.12,
  "bestQuality": 1.0,
  "averageQuality": 0.85,
  "qualityRanges": [
    {"range": "0.9 < Q <= 1.0", "count": 80000},
    {"range": "0.8 < Q <= 0.9", "count": 30000}
  ]
}
```

**ImportPpcf**
```json
// 请求（几何 ppcf，使用 openDocument）
{"command": "ImportPpcf", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf", "importMode": "geometry"}}

// 请求（网格 ppcf，使用 importMesh）
{"command": "ImportPpcf", "params": {"ppcfPath": "F:/Project/T1230/mesh.ppcf", "importMode": "mesh"}}

// 响应
{"success": true, "message": "ppcf 已导入，顶层数据已刷新。...", "ppcfPath": "F:/Project/T1230/T1230.ppcf", "importMode": "geometry"}
```

---

### 4. 数据查询

**GetVolumeListNames**
```json
// 请求（从会话）
{"command": "GetVolumeListNames", "params": {"sessionId": "geom_1"}}

// 请求（从 ppcf）
{"command": "GetVolumeListNames", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 响应
{"success": true, "volumeNames": ["fluid", "solid1", "solid2"]}
```

**GetFaceGroupNamesByVolume**
```json
// 请求
{"command": "GetFaceGroupNamesByVolume", "params": {"sessionId": "geom_1", "volumeName": "fluid"}}
// 或从 ppcf
{"command": "GetFaceGroupNamesByVolume", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf", "volumeName": "fluid"}}

// 响应
{"success": true, "volumeName": "fluid", "faceGroupNames": ["inlet", "outlet", "wall"]}
```

**RefreshSessionData**
```json
// 请求
{"command": "RefreshSessionData", "params": {"sessionId": "geom_1"}}

// 响应
{"success": true, "message": "顶层数据已刷新"}
```

---

### 5. 几何可视化

**ShowGeometry**
```json
// 请求
{"command": "ShowGeometry", "params": {"sessionId": "geom_1"}}

// 响应
{"success": true, "message": "几何可视化窗口正在启动，请稍候。窗口出现后可使用 RenderVolumesWithOpacity 等命令。", "sessionId": "geom_1"}
```

---

### 6. 网格可视化

**ShowMesh**
```json
// 请求（传入 ppcf 路径）
{"command": "ShowMesh", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 请求（不传 ppcfPath，复用 ExecuteMeshGeneration 后的 GeometryAPI）
{"command": "ShowMesh", "params": {}}

// 响应
{"success": true, "message": "网格可视化窗口正在启动，请稍候。...", "ppcfPath": "F:/Project/T1230/T1230.ppcf"}
```

**GetMeshWindowHandle**
```json
// 请求
{"command": "GetMeshWindowHandle", "params": {}}

// 响应
{"success": true, "windowHandle": 12345678, "windowTitle": "Mesh Viewer"}
```

**ResetMeshCamera**
```json
// 请求
{"command": "ResetMeshCamera", "params": {}}

// 响应
{"success": true, "message": "相机已重置"}
```

**RenderMesh**
```json
// 请求
{"command": "RenderMesh", "params": {}}

// 响应
{"success": true, "message": "渲染完成"}
```

**SetMeshSize**
```json
// 请求
{"command": "SetMeshSize", "params": {"width": 800, "height": 600}}

// 响应
{"success": true, "message": "窗口大小已设置"}
```

**ToggleMeshEdges**
```json
// 请求（显示边线）
{"command": "ToggleMeshEdges", "params": {"enable": true}}

// 请求（隐藏边线）
{"command": "ToggleMeshEdges", "params": {"enable": false}}

// 响应
{"success": true, "message": "边线显示已更新"}
```

**SetMeshRepresentation**
```json
// 请求（表面模式）
{"command": "SetMeshRepresentation", "params": {"representation": "surface"}}

// 请求（线框模式）
{"command": "SetMeshRepresentation", "params": {"representation": "wireframe"}}

// 请求（点模式）
{"command": "SetMeshRepresentation", "params": {"representation": "points"}}

// 响应
{"success": true, "message": "显示模式已更新"}
```

**ToggleMeshWireframe**
```json
// 请求
{"command": "ToggleMeshWireframe", "params": {"enable": true}}

// 响应
{"success": true, "message": "线框模式已更新"}
```

**ToggleMeshPoints**
```json
// 请求
{"command": "ToggleMeshPoints", "params": {"enable": true}}

// 响应
{"success": true, "message": "点显示已更新"}
```

**SetMeshTransparency**
```json
// 请求
{"command": "SetMeshTransparency", "params": {"opacity": 0.8}}

// 响应
{"success": true, "message": "透明度已设置"}
```

**ToggleMeshColorByGroup**
```json
// 请求
{"command": "ToggleMeshColorByGroup", "params": {"enable": true}}

// 响应
{"success": true, "message": "按组着色已更新"}
```

---

### 7. 体可视化

**GetVolumeWindowHandle**
```json
// 请求
{"command": "GetVolumeWindowHandle", "params": {}}

// 响应
{"success": true, "windowHandle": 12345678}
```

**ResetVolumeCamera**
```json
// 请求
{"command": "ResetVolumeCamera", "params": {}}

// 响应
{"success": true, "message": "相机已重置"}
```

**RenderVolume**
```json
// 请求
{"command": "RenderVolume", "params": {}}

// 响应
{"success": true, "message": "渲染完成"}
```

**SetVolumeSize**
```json
// 请求
{"command": "SetVolumeSize", "params": {"width": 800, "height": 600}}

// 响应
{"success": true, "message": "窗口大小已设置"}
```

**RenderVolumesWithOpacity**
```json
// 请求
{"command": "RenderVolumesWithOpacity", "params": {"volumeName": "fluid", "opacity": 0.5}}

// 响应
{"success": true, "message": "体渲染完成"}
```

---

## 流程示例

### 示例 1：完整几何 + 网格流程（无 session 复用）

```json
// 1. 导入几何
{"command": "ImportGeometryModel", "params": {"jsonPath": "F:/Project/T1230/T1230.json"}}
// 返回: {"success": true, "sessionId": "geom_1"}

// 2. 几何处理
{"command": "ExecuteGeometryProcessing", "params": {"sessionId": "geom_1"}}

// 3. 几何识别匹配
{"command": "ExecuteGeometryMatching", "params": {"sessionId": "geom_1", "jsonPath": "F:/Project/T1230/T1230.json"}}

// 4. 保存 ppcf（有啥保存啥）
{"command": "SavePpcf", "params": {"savePath": "F:/Project/T1230/T1230.ppcf"}}

// 5. 网格划分（需 ppcf 已存在）
{"command": "ExecuteMeshGeneration", "params": {"jsonPath": "F:/Project/T1230/T1230.json", "ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 6. 显示网格（不传 ppcfPath 则复用 ExecuteMeshGeneration 的 GeometryAPI）
{"command": "ShowMesh", "params": {}}

// 7. 保存 ppcf（网格划分后，有啥保存啥）
{"command": "SavePpcf", "params": {"savePath": "F:/Project/T1230/T1230.ppcf"}}

// 8. 关闭会话
{"command": "CloseSession", "params": {"sessionId": "geom_1"}}
```

---

### 示例 2：几何 + 网格流程（复用 session）

```json
// 1. 导入几何
{"command": "ImportGeometryModel", "params": {"jsonPath": "F:/Project/T1230/T1230.json"}}
// 返回: {"success": true, "sessionId": "geom_1"}

// 2. 几何处理
{"command": "ExecuteGeometryProcessing", "params": {"sessionId": "geom_1"}}

// 3. 几何识别匹配
{"command": "ExecuteGeometryMatching", "params": {"sessionId": "geom_1", "jsonPath": "F:/Project/T1230/T1230.json"}}

// 4. 网格划分（传 sessionId 复用 GeometryAPI，可不保存 ppcf）
{"command": "ExecuteMeshGeneration", "params": {"jsonPath": "F:/Project/T1230/T1230.json", "sessionId": "geom_1"}}

// 5. 显示网格（不传 ppcfPath，复用 ExecuteMeshGeneration 的 GeometryAPI）
{"command": "ShowMesh", "params": {}}

// 6. 保存 ppcf（有啥保存啥）
{"command": "SavePpcf", "params": {"savePath": "F:/Project/T1230/T1230.ppcf"}}
```

---

### 示例 3：ImportPpcf 导入并刷新顶层数据

```json
// 1. 导入 ppcf，顶层数据刷新为 ppcf 内容
{"command": "ImportPpcf", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 2. 后续查询、显示、质量检查均可复用（ppcfPath 或 ShowMesh 不传路径）
{"command": "GetVolumeListNames", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}
{"command": "ShowMesh", "params": {}}
{"command": "GetMeshQuality", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}
```

---

### 示例 4：从已有 ppcf 显示网格

```json
// 直接打开已有 ppcf 显示网格
{"command": "ShowMesh", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 获取网格质量
{"command": "GetMeshQuality", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}
```

---

### 示例 5：查询体与面组

```json
// 从会话获取体列表
{"command": "GetVolumeListNames", "params": {"sessionId": "geom_1"}}

// 从 ppcf 获取体列表
{"command": "GetVolumeListNames", "params": {"ppcfPath": "F:/Project/T1230/T1230.ppcf"}}

// 获取某体的面组名称
{"command": "GetFaceGroupNamesByVolume", "params": {"sessionId": "geom_1", "volumeName": "fluid"}}
```

---

### 示例 6：网格可视化控制

```json
// 显示网格后，调整显示
{"command": "RenderMesh", "params": {}}
{"command": "SetMeshSize", "params": {"width": 800, "height": 600}}
{"command": "ToggleMeshEdges", "params": {"enable": true}}
{"command": "SetMeshRepresentation", "params": {"representation": "wireframe"}}
{"command": "SetMeshTransparency", "params": {"opacity": 0.8}}
```

---

## 启动与连接

### 启动服务端

```bash
# 默认端口 12345
MappingGeometry.exe

# 指定端口
MappingGeometry.exe -socket -port 54321
```

### 测试客户端

项目提供 `SocketTestClient` 用于测试：

1. 编译后运行 `build/bin/Debug/SocketTestClient.exe`
2. 输入主机（默认 localhost）和端口（默认 12345）
3. 点击「连接」
4. 选择命令、填写参数后点击「发送」

---

## SavePpcf 说明

`SavePpcf` 统一保存 ppcf 文件，**有啥保存啥**：
- 有几何数据 → 保存几何
- 有几何 + 网格数据 → 保存几何 + 网格

数据来源优先级：ExecuteMeshGeneration / ImportPpcf 的网格会话 > 几何 session（ExecuteMeshGeneration 带 sessionId 时）> 任意几何会话。

---

## 注意事项

1. **会话生命周期**：`sessionId` 在 `ImportGeometryModel` 成功后返回，`CloseSession` 后失效。
2. **ppcfPath 与 sessionId**：`ExecuteMeshGeneration` 传 `sessionId` 时复用几何 session，可不传 `ppcfPath`；不传 `sessionId` 时需 `ppcfPath` 指向已存在的几何 ppcf。
3. **ShowMesh 两种模式**：传 `ppcfPath` 时直接打开该文件；不传时复用 `ExecuteMeshGeneration` 或 `ImportPpcf` 后的 GeometryAPI。
4. **ImportPpcf**：导入 ppcf 后刷新顶层数据，后续 GetVolumeListNames、ShowMesh、GetMeshQuality 等可使用该 ppcfPath 或复用当前会话。会覆盖之前的 mesh 会话。
5. **params 格式**：`params` 必须为 JSON 对象，不能为 `null`；空参数可传 `{}`。
