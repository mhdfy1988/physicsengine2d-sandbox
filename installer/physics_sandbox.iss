[Setup]
AppId={{9E97AB4A-16A8-4B6C-AF2D-17D571D98650}
AppName=Physics Sandbox
AppVersion={#GetStringParam('AppVersion|1.0.0')}
DefaultDirName={autopf}\PhysicsSandbox
DefaultGroupName=Physics Sandbox
OutputDir=dist
OutputBaseFilename=PhysicsSandbox_Setup_{#GetStringParam('AppVersion|1.0.0')}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
WizardStyle=modern

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create desktop icon"; GroupDescription: "Additional icons:"

[Files]
Source: "bin\physics_sandbox.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "sandbox_ui.ini"; DestDir: "{app}"; Flags: onlyifdoesntexist

[Icons]
Name: "{group}\Physics Sandbox"; Filename: "{app}\physics_sandbox.exe"
Name: "{autodesktop}\Physics Sandbox"; Filename: "{app}\physics_sandbox.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\physics_sandbox.exe"; Description: "Launch Physics Sandbox"; Flags: nowait postinstall skipifsilent

