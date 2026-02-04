@echo off
chcp 65001 >nul 2>&1
echo Creating portable single-file exe using 360压缩...
echo.

REM 检查360压缩是否存在（尝试多个可能的路径）
set "ZIP360="
if exist "C:\Program Files\360\360zip\360zip.exe" set "ZIP360=C:\Program Files\360\360zip\360zip.exe"
if exist "C:\Program Files (x86)\360\360zip\360zip.exe" set "ZIP360=C:\Program Files (x86)\360\360zip\360zip.exe"
if exist "%ProgramFiles%\360\360zip\360zip.exe" set "ZIP360=%ProgramFiles%\360\360zip\360zip.exe"
if exist "%ProgramFiles(x86)%\360\360zip\360zip.exe" set "ZIP360=%ProgramFiles(x86)%\360\360zip\360zip.exe"
if exist "C:\Program Files\360\360zip\360zipcmd.exe" set "ZIP360=C:\Program Files\360\360zip\360zipcmd.exe"
if exist "C:\Program Files (x86)\360\360zip\360zipcmd.exe" set "ZIP360=C:\Program Files (x86)\360\360zip\360zipcmd.exe"

REM 如果还是找不到，尝试在PATH中查找
if "%ZIP360%"=="" (
    where 360zip.exe >nul 2>&1
    if %errorlevel%==0 (
        for /f "delims=" %%i in ('where 360zip.exe') do set "ZIP360=%%i"
    )
)
if "%ZIP360%"=="" (
    where 360zipcmd.exe >nul 2>&1
    if %errorlevel%==0 (
        for /f "delims=" %%i in ('where 360zipcmd.exe') do set "ZIP360=%%i"
    )
)

if "%ZIP360%"=="" (
    echo Error: 360压缩 not found!
    echo Please make sure 360压缩 is installed
    echo Common installation paths:
    echo   C:\Program Files\360\360zip\
    echo   C:\Program Files (x86)\360\360zip\
    echo.
    echo If installed elsewhere, please modify the script
    echo.
    pause
    exit /b 1
)

echo Found 360压缩 at: %ZIP360%
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

REM 360压缩创建自解压exe的方法
REM 方法1: 先创建ZIP文件，然后转换为自解压exe
"%ZIP360%" a -tzip -mx9 "temp_package.zip" "temp_package\*" >nul
if %errorlevel% neq 0 (
    echo Error: Failed to create zip archive
    echo Trying alternative method...
    
    REM 方法2: 直接创建自解压exe（如果360压缩支持）
    "%ZIP360%" a -sfx -mx9 "installer\MappingGeometry_Portable.exe" "temp_package\*" >nul
    if %errorlevel% neq 0 (
        echo Error: Failed to create self-extracting exe
        echo.
        echo Note: 360压缩 may not support command-line SFX creation
        echo Creating regular ZIP file instead...
        copy /Y "temp_package.zip" "installer\MappingGeometry_Portable.zip" >nul
        echo.
        echo Done! ZIP archive created: installer\MappingGeometry_Portable.zip
        echo Users will need to extract it manually before running MappingGeometry.exe
        rmdir /s /q temp_package 2>nul
        del /q temp_package.zip 2>nul
        pause
        exit /b 1
    )
) else (
    REM 如果ZIP创建成功，尝试转换为自解压exe
    REM 360压缩可能需要手动操作，这里我们创建一个批处理文件来帮助用户
    echo.
    echo Note: 360压缩 command-line may not fully support SFX creation
    echo Creating ZIP file and helper script...
    
    REM 创建辅助脚本，用户可以用360压缩手动创建自解压exe
    (
        echo @echo off
        echo echo Converting ZIP to self-extracting exe...
        echo echo.
        echo echo Please use 360压缩 GUI to convert:
        echo echo   1. Open installer\MappingGeometry_Portable.zip with 360压缩
        echo echo   2. Click "Tools" -^> "Create Self-Extracting Archive"
        echo echo   3. Set "Extract to" as: %%TEMP%%\MappingGeometry
        echo echo   4. Set "Run program" as: MappingGeometry.exe
        echo echo   5. Save as: MappingGeometry_Portable.exe
        echo echo.
        echo pause
    ) > installer\convert_to_sfx.bat
    
    copy /Y "temp_package.zip" "installer\MappingGeometry_Portable.zip" >nul
    echo.
    echo Done! ZIP archive created: installer\MappingGeometry_Portable.zip
    echo Helper script created: installer\convert_to_sfx.bat
    echo.
    echo To create self-extracting exe:
    echo   1. Open installer\MappingGeometry_Portable.zip with 360压缩
    echo   2. Use "Tools" -^> "Create Self-Extracting Archive"
    echo   3. Or run installer\convert_to_sfx.bat for instructions
)

REM 清理临时文件
rmdir /s /q temp_package 2>nul
del /q temp_package.zip 2>nul

echo.
pause

