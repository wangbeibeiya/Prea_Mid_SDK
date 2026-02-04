@echo off
chcp 65001 >nul 2>&1
echo Creating portable single-file exe...
echo.

REM 检查7-Zip是否存在（尝试多个可能的路径）
set "SEVENZIP="
if exist "C:\Program Files\7-Zip\7z.exe" set "SEVENZIP=C:\Program Files\7-Zip\7z.exe"
if exist "C:\Program Files (x86)\7-Zip\7z.exe" set "SEVENZIP=C:\Program Files (x86)\7-Zip\7z.exe"
if exist "%ProgramFiles%\7-Zip\7z.exe" set "SEVENZIP=%ProgramFiles%\7-Zip\7z.exe"
if exist "%ProgramFiles(x86)%\7-Zip\7z.exe" set "SEVENZIP=%ProgramFiles(x86)%\7-Zip\7z.exe"

REM 如果还是找不到，尝试在PATH中查找
if "%SEVENZIP%"=="" (
    where 7z.exe >nul 2>&1
    if %errorlevel%==0 (
        for /f "delims=" %%i in ('where 7z.exe') do set "SEVENZIP=%%i"
    )
)

if "%SEVENZIP%"=="" (
    echo Error: 7-Zip not found!
    echo Please install 7-Zip from: https://www.7-zip.org/
    echo Or modify the script to point to your 7-Zip installation path
    echo.
    pause
    exit /b 1
)

echo Found 7-Zip at: %SEVENZIP%
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
echo Creating self-extracting exe file...

REM 创建7-Zip压缩包
"%SEVENZIP%" a -t7z -mx9 -mmt=on "temp_package.7z" "temp_package\*" >nul
if %errorlevel% neq 0 (
    echo Error: Failed to create 7z archive
    pause
    exit /b 1
)

REM 创建SFX配置文件
(
echo ;!@Install@!UTF-8!
echo Title="MappingGeometry Portable"
echo BeginPrompt="Extracting MappingGeometry Portable..."
echo ExtractPath="%%TEMP%%\MappingGeometry"
echo RunProgram="MappingGeometry.exe"
echo ;!@InstallEnd@!
) > temp_package.txt

REM 查找SFX模块
set "SFX_MODULE="
if exist "C:\Program Files\7-Zip\SFX\7z.sfx" set "SFX_MODULE=C:\Program Files\7-Zip\SFX\7z.sfx"
if exist "C:\Program Files (x86)\7-Zip\SFX\7z.sfx" set "SFX_MODULE=C:\Program Files (x86)\7-Zip\SFX\7z.sfx"
if exist "%ProgramFiles%\7-Zip\SFX\7z.sfx" set "SFX_MODULE=%ProgramFiles%\7-Zip\SFX\7z.sfx"
if exist "%ProgramFiles(x86)%\7-Zip\SFX\7z.sfx" set "SFX_MODULE=%ProgramFiles(x86)%\7-Zip\SFX\7z.sfx"

REM 合并SFX模块、配置文件和压缩包
if "%SFX_MODULE%"=="" (
    echo Warning: SFX module not found, creating regular 7z archive instead
    copy /Y "temp_package.7z" "installer\MappingGeometry_Portable.7z" >nul
    echo.
    echo Done! Archive created: installer\MappingGeometry_Portable.7z
    echo Note: This is a regular archive, not a self-extracting exe
    echo Users will need to extract it manually before running MappingGeometry.exe
) else (
    copy /B "%SFX_MODULE%" + "temp_package.txt" + "temp_package.7z" "installer\MappingGeometry_Portable.exe" >nul
    if %errorlevel% neq 0 (
        echo Error: Failed to create self-extracting exe
        pause
        exit /b 1
    )
    echo.
    echo Done! Portable exe created: installer\MappingGeometry_Portable.exe
    echo This is a self-extracting exe that will extract and run automatically
)

REM 清理临时文件
rmdir /s /q temp_package 2>nul
del /q temp_package.7z 2>nul
del /q temp_package.txt 2>nul

echo.
pause
