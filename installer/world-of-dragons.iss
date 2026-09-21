; =============================================================================
;  World of Dragons - script do instalador (Inno Setup 6)
;
;  Como gerar o instalador:
;    1. Compile o jogo em src/ (gera src/WorldOfDragons.exe)
;    2. Abra este arquivo no Inno Setup Compiler
;    3. Build > Compile (Ctrl+F9)
;    O instalador sai em installer/Output/WorldOfDragons_setup.exe
;
;  Layout instalado:
;    {app}\bin\      executavel + DLLs do GTK/SDL
;    {app}\lib\      loaders de imagem do GTK
;    {app}\share\    temas, icones e schemas do GTK
;    {app}\assets\   imagens, sons, CSS e interface
;    {app}\files\    dados do jogo
;    {app}\accounts\ saves do jogador (criada em tempo de execucao)
;
; =============================================================================

#define AppName      "World of Dragons"
#define AppVersion   "2.0.0"
#define AppPublisher "Dhemerson Sousa"
#define AppURL       "https://github.com/Rebornned/world-of-dragons"
#define AppExeName   "WorldOfDragons.exe"
#define AppIcon      "T_dragons_ico.ico"

; Raiz do repositorio, relativa a este script
#define Root ".."

[Setup]
; Identificador unico do aplicativo. Nunca altere: e por ele que o Windows
; reconhece atualizacoes e desinstalacoes de versoes anteriores.
AppId={{93BF1E97-589E-4657-9B51-CC0C3F6F4C75}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases

; Instalacao por usuario, sem pedir permissao de administrador.
; Com PrivilegesRequired=lowest, {autopf} aponta para
; %LOCALAPPDATA%\Programs, onde o jogo pode gravar os saves livremente.
PrivilegesRequired=lowest
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes

; Apenas Windows 64 bits (as DLLs sao do MinGW64)
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Aparencia
SetupIconFile={#Root}\assets\img_files\{#AppIcon}
UninstallDisplayIcon={app}\assets\img_files\{#AppIcon}
UninstallDisplayName={#AppName}
WizardStyle=modern

; Saida
OutputDir=Output
OutputBaseFilename=WorldOfDragons_setup
Compression=lzma2/max
SolidCompression=yes

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Dirs]
; Pasta de saves. O jogo tambem a cria sozinho, mas ja deixamos pronta.
Name: "{app}\accounts"

[Files]
; Executavel e DLLs juntos em bin\ (ver explicacao no topo do arquivo)
Source: "{#Root}\src\{#AppExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#Root}\dlls\*.dll";        DestDir: "{app}\bin"; Flags: ignoreversion

; Runtime do GTK. Os .dll.a sao bibliotecas de linkagem, inuteis em tempo de
; execucao, e o validate.py e ferramenta de desenvolvimento.
Source: "{#Root}\lib\*";   DestDir: "{app}\lib";   Excludes: "*.dll.a,desktop.ini"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Root}\share\*"; DestDir: "{app}\share"; Excludes: "*.py,desktop.ini";    Flags: ignoreversion recursesubdirs createallsubdirs

; Assets do jogo. Ficam de fora as capturas do README, a fonte (instalada
; no sistema abaixo), scripts .bat antigos e arquivos de backup do Glade.
Source: "{#Root}\assets\*"; DestDir: "{app}\assets"; Excludes: "\screenshots,\fonts,*.bat,*~,desktop.ini"; Flags: ignoreversion recursesubdirs createallsubdirs

; Dados do jogo: dragoes, ataques e curva de niveis
Source: "{#Root}\files\*"; DestDir: "{app}\files"; Flags: ignoreversion recursesubdirs createallsubdirs

; Licencas
Source: "{#Root}\LICENSE";   DestDir: "{app}"; Flags: ignoreversion
Source: "{#Root}\ASSETS.md"; DestDir: "{app}"; Flags: ignoreversion

; Fonte da interface. Em instalacao sem administrador, {autofonts} aponta
; para as fontes do usuario (Windows 10 1803 ou superior). Sem o flag
; uninsneveruninstall, a fonte e removida junto na desinstalacao.
Source: "{#Root}\assets\fonts\Pixellari.ttf"; DestDir: "{autofonts}"; FontInstall: "Pixellari"; Flags: onlyifdoesntexist

[Icons]
; WorkingDir e obrigatorio: o jogo abre assets, dados e saves por caminhos
; relativos (..\assets, ..\files, ..\accounts), resolvidos a partir da pasta
; de trabalho, e nao da pasta do executavel.
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; IconFilename: "{app}\assets\img_files\{#AppIcon}"
Name: "{autodesktop}\{#AppName}";  Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; IconFilename: "{app}\assets\img_files\{#AppIcon}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#AppExeName}"; WorkingDir: "{app}\bin"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove os saves criados pelo jogo, que o desinstalador nao conhece
Type: filesandordirs; Name: "{app}\accounts"