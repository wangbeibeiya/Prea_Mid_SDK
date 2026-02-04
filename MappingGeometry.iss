; Inno Setup 安装脚本
; 用于打包 MappingGeometry 程序及其所有依赖文件

#define MyAppName "MappingGeometry"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Pera CFD SDK"
#define MyAppURL "https://www.peracfd.com/"
#define MyAppExeName "MappingGeometry.exe"

[Setup]
; 应用程序基本信息
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=
OutputDir=installer
OutputBaseFilename=MappingGeometry_Setup
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; 主程序文件
Source: "build\bin\Debug\MappingGeometry.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\MappingGeometry.pdb"; DestDir: "{app}"; Flags: ignoreversion

; SDK核心DLL文件
Source: "build\bin\Debug\PFBase.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\PFGeometry.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\PFMesh.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\PFPostProcess.dll"; DestDir: "{app}"; Flags: ignoreversion

; 第三方依赖DLL文件
Source: "build\bin\Debug\jsoncpp.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\LicenseDll.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\model_database.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\openNURBS3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\pthreads.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\xyssl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\bin\Debug\fluidModel.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt插件目录（递归包含所有子目录）
Source: "build\bin\Debug\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs

; bin目录下的工具程序（可选）
Source: "build\bin\Debug\bin\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs

; remote目录（可选）
Source: "build\bin\Debug\remote\*"; DestDir: "{app}\remote"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
// 自定义安装后处理过程
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    // 可以在这里添加安装后的自定义操作
    // 例如：创建配置文件、注册表项等
  end;
end;
