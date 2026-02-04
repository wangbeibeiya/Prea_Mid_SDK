# 打包说明

## 便携式单文件exe创建方法

### 方法1：使用360压缩（推荐，最简单）

1. **运行脚本创建ZIP文件**
   ```batch
   create_portable_exe_simple.bat
   ```

2. **使用360压缩创建自解压exe**
   - 打开生成的 `installer\MappingGeometry_Portable.zip`
   - 点击"工具" → "制作自解压文件"
   - 设置"解压路径"为：`%TEMP%\MappingGeometry`
   - 设置"解压后运行"为：`MappingGeometry.exe`
   - 保存为：`MappingGeometry_Portable.exe`

3. **输出文件**
   - `installer\MappingGeometry_Portable.exe`

### 方法2：使用7-Zip

1. **安装7-Zip**
   - 下载地址：https://www.7-zip.org/
   - 安装到默认路径：`C:\Program Files\7-Zip\`

2. **运行脚本**
   ```batch
   create_portable_exe.bat
   ```

3. **输出文件**
   - `installer\MappingGeometry_Portable.exe`

### 方法3：使用WinRAR

1. **确保已安装WinRAR**

2. **运行脚本**
   ```batch
   create_portable_exe_winrar.bat
   ```

3. **输出文件**
   - `installer\MappingGeometry_Portable.exe`

### 方法4：使用Inno Setup（安装程序）

1. **安装Inno Setup Compiler**
   - 下载地址：https://jrsoftware.org/isdl.php

2. **打开脚本**
   - 打开 `MappingGeometry.iss`

3. **编译**
   - 点击"编译"按钮（或按F9）

4. **输出文件**
   - `installer\MappingGeometry_Setup.exe`

## 文件说明

- **MappingGeometry_Portable.exe** - 便携式单文件exe，无需安装，双击运行
- **MappingGeometry_Portable.zip** - ZIP压缩包，需要手动解压
- **MappingGeometry_Setup.exe** - 安装程序，需要安装到系统目录

## 注意事项

- 便携式exe会在临时目录解压并运行
- 安装程序需要管理员权限
- 确保所有依赖DLL文件都在 `build\bin\Debug` 目录下
- 360压缩方法最简单，适合大多数用户

