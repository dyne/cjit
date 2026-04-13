@echo off
setlocal EnableExtensions EnableDelayedExpansion

if "%~1"=="" goto :usage

set "ACTION=%~1"
set "ROOT=%~2"
set "SOURCE_LIST=%~3"
set "PREFIX=%CJIT_PREFIX%"
set "VERSION=%CJIT_VERSION%"
set "CURRENT_YEAR=%CJIT_CURRENT_YEAR%"

if "%ROOT%"=="" goto :usage

call :find_vsdevcmd || exit /b 1
call :load_vs || exit /b 1

if /I "%ACTION%"=="tinycc" goto :tinycc
if /I "%ACTION%"=="cjit" goto :cjit
if /I "%ACTION%"=="cjit-ar" goto :cjit_ar

echo Unknown action: %ACTION% 1>&2
exit /b 1

:usage
echo usage: build\win-msvc.cmd ^<tinycc^|cjit^|cjit-ar^> root [source-list] 1>&2
exit /b 1

:find_vsdevcmd
for %%R in ("%ProgramFiles%\Microsoft Visual Studio" "%ProgramFiles(x86)%\Microsoft Visual Studio") do (
  if exist "%%~fR" (
    for /d %%Y in ("%%~fR\*") do (
      for /d %%E in ("%%~fY\*") do (
        if exist "%%~fE\Common7\Tools\VsDevCmd.bat" (
          set "VSDEVCMD=%%~fE\Common7\Tools\VsDevCmd.bat"
          exit /b 0
        )
      )
    )
  )
)
echo VsDevCmd.bat not found 1>&2
exit /b 1

:load_vs
set "PATH=%SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SystemRoot%\System32\WindowsPowerShell\v1.0\;%PATH%"
call "%VSDEVCMD%" -arch=x64 -host_arch=x64 >nul || exit /b 1
if defined CL (
  set "CL=/FS %CL%"
) else (
  set "CL=/FS"
)
exit /b 0

:tinycc
cd /d "%ROOT%\lib\tinycc\win32" || exit /b 1
call build-tcc.bat -clean || exit /b 1
call build-tcc.bat -c cl -t 64 || exit /b 1
if not exist "lib\libtcc1.a" exit /b 1
if not exist "libtcc.lib" exit /b 1
if not exist "libtcc.dll" exit /b 1
copy /Y "lib\libtcc1.a" "%ROOT%\lib\tinycc\libtcc1.a" >nul || exit /b 1
copy /Y "libtcc.lib" "%ROOT%\lib\tinycc\libtcc.lib" >nul || exit /b 1
copy /Y "libtcc.dll" "%ROOT%\libtcc.dll" >nul || exit /b 1
exit /b 0

:cjit
if "%SOURCE_LIST%"=="" (
  echo No source list passed to cjit build 1>&2
  exit /b 1
)
if not exist "%ROOT%\build\win-msvc" mkdir "%ROOT%\build\win-msvc" || exit /b 1
set "RSP=%ROOT%\build\win-msvc\cjit.rsp"
break > "%RSP%" || exit /b 1
for /f "usebackq delims=" %%I in ("%SOURCE_LIST%") do >> "%RSP%" echo %%~I

:cjit_link
cl /nologo /O2 /W2 /MT /GS- ^
  /DCJIT_BUILD_WIN ^
  /DTCC_TARGET_PE ^
  /DTCC_TARGET_X86_64 ^
  /DPREFIX=\"%PREFIX%\" ^
  /DVERSION=\"%VERSION%\" ^
  /DCURRENT_YEAR=\"%CURRENT_YEAR%\" ^
  /I"%ROOT%\src" /I"%ROOT%\lib\tinycc" /I"%ROOT%\lib\muntarfs" ^
  /Fo"%ROOT%\build\win-msvc\\" ^
  /Fe"%ROOT%\cjit.exe" ^
  @"%RSP%" "%ROOT%\lib\tinycc\libtcc.c" /link /nologo advapi32.lib shlwapi.lib rpcrt4.lib || exit /b 1
exit /b 0

:cjit_ar
if not exist "%ROOT%\build\win-msvc" mkdir "%ROOT%\build\win-msvc" || exit /b 1
cl /nologo /O2 /W2 /MT /GS- ^
  /DCJIT_BUILD_WIN ^
  /DCJIT_AR_MAIN ^
  /DTCC_TARGET_PE ^
  /DTCC_TARGET_X86_64 ^
  /DPREFIX=\"%PREFIX%\" ^
  /DVERSION=\"%VERSION%\" ^
  /DCURRENT_YEAR=\"%CURRENT_YEAR%\" ^
  /I"%ROOT%\src" /I"%ROOT%\lib\tinycc" /I"%ROOT%\lib\muntarfs" ^
  /Fo"%ROOT%\build\win-msvc\\" ^
  /Fe"%ROOT%\cjit-ar.exe" ^
  "%ROOT%\src\cjit-ar.c" "%ROOT%\lib\tinycc\libtcc.c" ^
  /link /nologo advapi32.lib shlwapi.lib rpcrt4.lib || exit /b 1
exit /b 0
