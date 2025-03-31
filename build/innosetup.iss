#define MyAppName "CJIT"
; #define MyAppVersion "1.0"
#define MyAppPublisher "Dyne.org foundation"
#define MyAppURL "https://dyne.org/docs/cjit"
#define MyAppExeName "cjit.exe"
#define MyAppAssocName "C Source Code"
#define MyAppAssocExt ".c"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt
#define MyBuildHome ".."
;  "C:\Users\runneradmin"
[Setup]
DisableStartupPrompt=yes
AppId={{424647AA-C490-4CE6-85E9-A988CB6D3089}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
; "ArchitecturesAllowed=x64compatible" specifies that Setup cannot run
; on anything but x64 and Windows 11 on Arm.
ArchitecturesAllowed=x64compatible
; "ArchitecturesInstallIn64BitMode=x64compatible" requests that the
; install be done in "64-bit mode" on x64 or Windows 11 on Arm,
; meaning it should use the native 64-bit Program Files directory and
; the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
; LicenseFile={#MyBuildHome}\LICENSES\GPL-3.0-or-later.txt
; InfoBeforeFile={#MyBuildHome}\README.md
; InfoAfterFile={#MyBuildHome}\docs\cjit.1
; Remove the following line to run in administrative install mode (install for all users).

PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename=cjit_innosetup
SolidCompression=yes
; WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#MyBuildHome}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyBuildHome}\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyBuildHome}\REUSE.toml"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyBuildHome}\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyBuildHome}\test\*"; DestDir: "{app}\test"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyBuildHome}\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyBuildHome}\LICENSES\*"; DestDir: "{app}\licenses"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  // If /ALLUSERS is passed, force admin mode
  if ExpandConstant('{param:ALLUSERS|false}') = 'true' then
  begin
	if not IsAdminLoggedOn() then
	begin
	  MsgBox('Administrator privileges are required for machine-wide installation.', mbError, MB_OK);
	  Result := False;
	end;
  end;
end;

function InitializeUninstall(): Boolean;
begin
  Result := True;
  // If uninstalling an ALLUSERS installation, require admin
  if IsAdminInstallMode() then
  begin
	if not IsAdminLoggedOn() then
	begin
	  MsgBox('Administrator privileges are required to uninstall a machine-wide installation.', mbError, MB_OK);
	  Result := False;
	end;
  end;
end;