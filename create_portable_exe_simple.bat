@echo off
chcp 65001 >nul 2>&1
echo Creating portable package...
echo.

REM 创建输出目录
if not exist "installer" mkdir installer

REM 创建临时打包目录
if exist "temp_package" rmdir /s /q temp_package
mkdir temp_package

echo Copying files...

REM 复制主程序
if exist "build\bin\Debug\MappingGeometry.exe" (
    copy /Y "build\bin\Debug\MappingGeometry.exe" "temp_package\" >nul
    echo   - MappingGeometry.exe
)
if exist "build\bin\Debug\MappingGeometry.pdb" (
    copy /Y "build\bin\Debug\MappingGeometry.pdb" "temp_package\" >nul
)

REM 复制DLL文件
set "DLL_COUNT=0"
for %%f in (
    "PFBase.dll" "PFGeometry.dll" "PFMesh.dll" "PFPostProcess.dll"
    "jsoncpp.dll" "LicenseDll.dll" "model_database.dll" "openNURBS3.dll"
    "pthreads.dll" "xyssl.dll" "zlib.dll" "Qt5Core.dll" "fluidModel.dll"
) do (
    if exist "build\bin\Debug\%%~f" (
        copy /Y "build\bin\Debug\%%~f" "temp_package\" >nul
        set /a DLL_COUNT+=1
    )
)
echo   - %DLL_COUNT% DLL files copied

REM 复制目录
if exist "build\bin\Debug\plugins" (
    xcopy /E /I /Y /Q "build\bin\Debug\plugins" "temp_package\plugins\" >nul
    echo   - plugins directory
)
if exist "build\bin\Debug\bin" (
    xcopy /E /I /Y /Q "build\bin\Debug\bin" "temp_package\bin\" >nul
    echo   - bin directory
)
if exist "build\bin\Debug\remote" (
    xcopy /E /I /Y /Q "build\bin\Debug\remote" "temp_package\remote\" >nul
    echo   - remote directory
)

echo.
echo Creating ZIP archive...

REM 尝试使用PowerShell创建ZIP（Windows内置，无需额外软件）
powershell -Command "Compress-Archive -Path 'temp_package\*' -DestinationPath 'installer\MappingGeometry_Portable.zip' -Force" >nul 2>&1

if %errorlevel% neq 0 (
    echo Error: Failed to create ZIP archive
    echo.
    echo Please manually create ZIP archive:
    echo   1. Right-click on "temp_package" folder
    echo   2. Select "Send to" -^> "Compressed (zipped) folder"
    echo   3. Rename it to "MappingGeometry_Portable.zip"
    echo   4. Move it to "installer" folder
    echo.
    echo Then use 360压缩 to create self-extracting exe:
    echo   1. Open MappingGeometry_Portable.zip with 360压缩
    echo   2. Click "Tools" -^> "Create Self-Extracting Archive"
    echo   3. Set extraction path and run program
    echo.
    pause
    exit /b 1
)

echo Done! ZIP archive created: installer\MappingGeometry_Portable.zip
echo.
echo Next steps to create self-extracting exe with 360压缩:
echo   1. Open "installer\MappingGeometry_Portable.zip" with 360压缩
echo   2. Click "工具" -^> "制作自解压文件"
echo   3. Set "解压路径" as: %%TEMP%%\MappingGeometry
echo   4. Set "解压后运行" as: MappingGeometry.exe
echo   5. Save as: MappingGeometry_Portable.exe
echo.

REM 清理临时文件
rmdir /s /q temp_package 2>nul

echo Package ready! You can now use 360压缩 to create self-extracting exe.
echo.
pause

