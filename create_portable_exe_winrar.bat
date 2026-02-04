@echo off
chcp 65001 >nul 2>&1
echo Creating portable single-file exe using WinRAR...
echo.

REM 检查WinRAR是否存在
set "WINRAR="
if exist "C:\Program Files\WinRAR\WinRAR.exe" set "WINRAR=C:\Program Files\WinRAR\WinRAR.exe"
if exist "C:\Program Files (x86)\WinRAR\WinRAR.exe" set "WINRAR=C:\Program Files (x86)\WinRAR\WinRAR.exe"
if exist "%ProgramFiles%\WinRAR\WinRAR.exe" set "WINRAR=%ProgramFiles%\WinRAR\WinRAR.exe"
if exist "%ProgramFiles(x86)%\WinRAR\WinRAR.exe" set "WINRAR=%ProgramFiles(x86)%\WinRAR\WinRAR.exe"

if "%WINRAR%"=="" (
    echo Error: WinRAR not found!
    echo Please install WinRAR or use the 7-Zip version
    echo.
    pause
    exit /b 1
)

echo Found WinRAR at: %WINRAR%
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

REM 创建SFX配置文件
(
echo Path=%%T
echo SavePath
echo Silent=1
echo Overwrite=1
echo Update=U
echo Setup=MappingGeometry.exe
echo Title=MappingGeometry Portable
) > temp_package.sfx

REM 使用WinRAR创建自解压exe
"%WINRAR%" a -sfx -z"temp_package.sfx" -ep1 -r -y "installer\MappingGeometry_Portable.exe" "temp_package\*" >nul
if %errorlevel% neq 0 (
    echo Error: Failed to create self-extracting exe
    pause
    exit /b 1
)

REM 清理临时文件
rmdir /s /q temp_package 2>nul
del /q temp_package.sfx 2>nul

echo.
echo Done! Portable exe created: installer\MappingGeometry_Portable.exe
echo This is a self-extracting exe that will extract and run automatically
echo.
pause

