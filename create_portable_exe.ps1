# PowerShell脚本：创建便携式单文件exe
# 使用7-Zip创建自解压exe文件

$ErrorActionPreference = "Stop"

# 配置参数
$SourceDir = "build\bin\Debug"
$OutputFile = "installer\MappingGeometry_Portable.exe"
$TempDir = "temp_package"
$7ZipPath = "C:\Program Files\7-Zip\7z.exe"  # 7-Zip路径，如果不在默认位置请修改

# 检查7-Zip是否存在
if (-not (Test-Path $7ZipPath)) {
    Write-Host "错误: 找不到7-Zip，请安装7-Zip或修改脚本中的路径" -ForegroundColor Red
    Write-Host "7-Zip下载地址: https://www.7-zip.org/" -ForegroundColor Yellow
    exit 1
}

# 创建输出目录
$OutputDir = Split-Path -Parent $OutputFile
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

# 创建临时打包目录
if (Test-Path $TempDir) {
    Remove-Item -Path $TempDir -Recurse -Force
}
New-Item -ItemType Directory -Path $TempDir -Force | Out-Null

Write-Host "正在复制文件到临时目录..." -ForegroundColor Green

# 复制主程序
Copy-Item "$SourceDir\MappingGeometry.exe" -Destination "$TempDir\" -Force
Copy-Item "$SourceDir\MappingGeometry.pdb" -Destination "$TempDir\" -Force

# 复制DLL文件
$DllFiles = @(
    "PFBase.dll", "PFGeometry.dll", "PFMesh.dll", "PFPostProcess.dll",
    "jsoncpp.dll", "LicenseDll.dll", "model_database.dll", "openNURBS3.dll",
    "pthreads.dll", "xyssl.dll", "zlib.dll", "Qt5Core.dll", "fluidModel.dll"
)

foreach ($dll in $DllFiles) {
    if (Test-Path "$SourceDir\$dll") {
        Copy-Item "$SourceDir\$dll" -Destination "$TempDir\" -Force
    }
}

# 复制plugins目录
if (Test-Path "$SourceDir\plugins") {
    Copy-Item "$SourceDir\plugins" -Destination "$TempDir\" -Recurse -Force
}

# 复制bin目录（如果需要）
if (Test-Path "$SourceDir\bin") {
    Copy-Item "$SourceDir\bin" -Destination "$TempDir\" -Recurse -Force
}

# 复制remote目录（如果需要）
if (Test-Path "$SourceDir\remote") {
    Copy-Item "$SourceDir\remote" -Destination "$TempDir\" -Recurse -Force
}

Write-Host "正在创建自解压exe文件..." -ForegroundColor Green

# 创建7-Zip压缩包
$ZipFile = "$TempDir.7z"
& $7ZipPath a -t7z -mx9 "$ZipFile" "$TempDir\*" | Out-Null

# 创建SFX配置文件
$SfxConfig = @"
;!@Install@!UTF-8!
Title="MappingGeometry Portable"
BeginPrompt="正在解压 MappingGeometry 便携版..."
ExtractPath="%TEMP%\MappingGeometry"
RunProgram="MappingGeometry.exe"
;!@InstallEnd@!
"@

$SfxConfigFile = "$TempDir.txt"
$SfxConfig | Out-File -FilePath $SfxConfigFile -Encoding UTF8 -NoNewline

# 创建SFX模块（需要7-Zip的SFX模块）
$SfxModule = "C:\Program Files\7-Zip\SFX\7z.sfx"
if (-not (Test-Path $SfxModule)) {
    Write-Host "警告: 找不到7-Zip SFX模块，将创建普通压缩包" -ForegroundColor Yellow
    Copy-Item $ZipFile -Destination $OutputFile -Force
} else {
    # 合并SFX模块、配置文件和压缩包
    $SfxConfigBytes = [System.IO.File]::ReadAllBytes($SfxConfigFile)
    $SfxModuleBytes = [System.IO.File]::ReadAllBytes($SfxModule)
    $ZipBytes = [System.IO.File]::ReadAllBytes($ZipFile)
    
    $AllBytes = $SfxModuleBytes + $SfxConfigBytes + $ZipBytes
    [System.IO.File]::WriteAllBytes($OutputFile, $AllBytes)
}

# 清理临时文件
Remove-Item -Path $TempDir -Recurse -Force
Remove-Item -Path $ZipFile -Force -ErrorAction SilentlyContinue
Remove-Item -Path $SfxConfigFile -Force -ErrorAction SilentlyContinue

Write-Host "完成! 便携式exe文件已创建: $OutputFile" -ForegroundColor Green

