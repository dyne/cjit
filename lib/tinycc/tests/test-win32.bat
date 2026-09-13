@echo off
setlocal
set CC=gcc
set TESTS=
if "%1"=="/?" goto :usage
goto :p0

:usage
echo usage: test-win32.bat [options...] [tests...]
echo options:
echo   -c compiler  reference compiler (gcc or clang)
echo   -p path      prepend path to PATH
echo tests          tests to run (default all)
echo requires:      make, gcc/clang, and sh in the PATH
exit /B 1

:p2
shift
:p1
shift
:p0
if "%1"=="-c" set CC=%2&&goto p2
if "%1"=="-p" set PATH=%2;%PATH%&&goto p2
if not "%1"=="" set TESTS=%TESTS% %1&&goto p1
if "%TESTS%"=="" set TESTS=all -k

set PATH=%CD%\..\win32;%PATH%

for /f "delims=-" %%a in ('tcc.exe -dumpmachine') do set ARCH=%%a
set ARCH=%ARCH:aarch=arm%
set MACH=%ARCH:i386=x86%
set MACH=%MACH:x86_64=amd64%
rem echo ARCH:%ARCH%  MACHINE:%MACH%

set CRTLIB=c:/windows/system32/msvcrt.dll
set LIBTCC=libtcc.dll
set LGCC=-lgcc
if %CC%==gcc goto :c2

set CRTLIB=msvcrt.lib
set LIBTCC=libtcc.lib
set LGCC=
if exist %CRTLIB% goto :c3
tcc -impdef msvcrt.dll
lib >nul /def:msvcrt.def /out:%CRTLIB% /machine:%MACH%
:c2
if not exist %CRTLIB% echo test-win32.bat: error: %CRTLIB% not found&&exit /b 1
:c3

set REF_LINK=-nostdlib msvcrt_start.c %LGCC% -lkernel32 %CRTLIB%
set CFG_MAK=..\config.mak
set CFG_H=..\config.h

echo>>%CFG_H% #define CC_NAME CC_%CC%
echo>>%CFG_H% #define GCC_MAJOR 15

if exist %CFG_MAK% del %CFG_MAK%
echo>>%CFG_MAK% CC = %CC%.exe
echo>>%CFG_MAK% CC_NAME = %CC%
echo>>%CFG_MAK% ARCH = %ARCH%
echo>>%CFG_MAK% CFLAGS = -Wall -O0
echo>>%CFG_MAK% LDFLAGS =
echo>>%CFG_MAK% LIBSUF = .lib
echo>>%CFG_MAK% prefix = $(TOP)/bin
echo>>%CFG_MAK% EXESUF = .exe
echo>>%CFG_MAK% DLLSUF = .dll
echo>>%CFG_MAK% TARGETOS = WIN32
echo>>%CFG_MAK% CONFIG_WIN32 = yes
echo>>%CFG_MAK% TOPSRC = $(TOP)
echo>>%CFG_MAK% SHELL = sh
echo>>%CFG_MAK% test.ref: CFLAGS += %REF_LINK%

if "%GMAKE%"=="" set GMAKE=make
%GMAKE% TCC_LOCAL=tcc.exe LIBTCC=win32/%LIBTCC% %TESTS%
