#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// 前向声明，避免在 RenderProcessor 中引入 VTK/Socket 头文件
class vtkActor;

/**
 * @brief 设置体名称与 mesh actor 的映射（供 RenderProcessor 调用）
 *
 * 不包含 Socket/VTK 完整头文件，避免与 RenderProcessor 的 Windows 头冲突。
 */
void SetVolumeActorMapping(const std::unordered_map<std::string, std::vector<vtkActor*>>& volumeActorMap);
