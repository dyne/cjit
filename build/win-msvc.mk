# Native windows build using the Microsoft Visual C toolchain.
# The actual MSVC work runs in a native cmd script to avoid MSYS path
# conversion issues around cl.exe and VsDevCmd.bat.

include build/init.mk

GENERATED_WIN_FILES := src/assets.c src/assets.h \
	src/embed_libtcc1.a.c src/embed_include.c \
	src/embed_tinycc_win32.c src/embed_win32ports.c

SHELL := C:\Program Files\Git\bin\bash.exe

CURDIR_WIN := $(subst /,\,$(CURDIR))
POWERSHELL := /c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe

MSVC_SOURCES := $(filter-out src/adapters/platform/library_resolver_posix.o,$(SOURCES))
MSVC_SOURCES += src/win-compat.o src/embed_tinycc_win32.o src/embed_win32ports.o
MSVC_SOURCES := $(MSVC_SOURCES:.o=.c)

all: win-msvc

tinycc-msvc:
	@mkdir -p build/win-msvc
	@"$(POWERSHELL)" -NoProfile -ExecutionPolicy Bypass -File build/win-msvc.ps1 -Action tinycc -Root "$(CURDIR_WIN)" -Prefix "$(PREFIX)" -Version "$(VERSION)" -CurrentYear "$(CURRENT_YEAR)"
	@test -r lib/tinycc/libtcc1.a
	@test -r lib/tinycc/libtcc.lib
	@test -r libtcc.dll

embed-win-msvc: tinycc-msvc
	$(info Embedding assets for Windows build)
	bash build/init-assets.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh lib/tinycc/win32/include tinycc_win32
	bash build/embed-asset-path.sh assets/win32ports
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h

${GENERATED_WIN_FILES}: embed-win-msvc

cjit.exe: embed-win-msvc
	@mkdir -p build/win-msvc
	@rm -f build/win-msvc/cjit-sources.txt
	@printf '%s\n' $(foreach source,$(MSVC_SOURCES),"$(CURDIR_WIN)\$(subst /,\,$(source))") > build/win-msvc/cjit-sources.txt
	@"$(POWERSHELL)" -NoProfile -ExecutionPolicy Bypass -File build/win-msvc.ps1 -Action cjit -Root "$(CURDIR_WIN)" -Prefix "$(PREFIX)" -Version "$(VERSION)" -CurrentYear "$(CURRENT_YEAR)" -SourceList "$(CURDIR_WIN)\build\win-msvc\cjit-sources.txt"

cjit-ar.exe: tinycc-msvc
	@mkdir -p build/win-msvc
	@"$(POWERSHELL)" -NoProfile -ExecutionPolicy Bypass -File build/win-msvc.ps1 -Action cjit-ar -Root "$(CURDIR_WIN)" -Prefix "$(PREFIX)" -Version "$(VERSION)" -CurrentYear "$(CURRENT_YEAR)"

win-msvc: cjit.exe cjit-ar.exe
	@rm -f .build_done*
	date | tee .build_done_win_msvc
