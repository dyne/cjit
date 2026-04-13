# Native windows build using the Microsoft Visual C toolchain.
#
# This build path uses the vendored TinyCC Windows batch builder, which
# already knows how to bootstrap TinyCC with cl.exe.

include build/init.mk

GENERATED_WIN_FILES := src/assets.c src/assets.h \
	src/embed_libtcc1.a.c src/embed_include.c \
	src/embed_tinycc_win32.c src/embed_win32ports.c

SHELL := C:\Program Files\Git\bin\bash.exe

CURDIR_WIN := $(shell cygpath -w "$(CURDIR)")
VSDEVCMD := $(strip $(shell for root in "/c/Program Files/Microsoft Visual Studio" "/c/Program Files (x86)/Microsoft Visual Studio"; do for path in "$$root"/*/*/Common7/Tools/VsDevCmd.bat; do [ -r "$$path" ] || continue; printf '%s\n' "$$path"; exit 0; done; done))

MSVC_SOURCES := $(filter-out src/adapters/platform/library_resolver_posix.o,$(SOURCES))
MSVC_SOURCES += src/win-compat.o src/embed_tinycc_win32.o src/embed_win32ports.o
MSVC_SOURCES := $(MSVC_SOURCES:.o=.c)

msvc_cppflags := /DCJIT_BUILD_WIN /DPREFIX=\"$(PREFIX)\" /DVERSION=\"$(VERSION)\" /DCURRENT_YEAR=\"$(CURRENT_YEAR)\"
msvc_cppflags += /I"$(CURDIR_WIN)\src" /I"$(CURDIR_WIN)\lib\tinycc" /I"$(CURDIR_WIN)\lib\muntarfs"
msvc_cflags := /nologo /O2 /W2 /MT /GS- $(msvc_cppflags)
msvc_ldflags := /link /nologo advapi32.lib shlwapi.lib rpcrt4.lib

all: win-msvc

check-vsdevcmd:
	@test -n "$(VSDEVCMD)" || { echo "VsDevCmd.bat not found" >&2; exit 1; }

tinycc-msvc: check-vsdevcmd
	@. build/vsdevcmd-env.sh && \
	/c/Windows/System32/cmd.exe /c "set \"PATH=%SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SystemRoot%\System32\WindowsPowerShell\v1.0\;%PATH%\" && call \"$$VSDEVCMD_WIN\" -arch=x64 -host_arch=x64 >nul && cd /d \"$(CURDIR_WIN)\lib\tinycc\win32\" && build-tcc.bat -clean && build-tcc.bat -c cl -t 64"
	@cp lib/tinycc/win32/lib/libtcc1.a lib/tinycc/libtcc1.a
	@cp lib/tinycc/win32/libtcc.lib lib/tinycc/libtcc.lib
	@cp lib/tinycc/win32/libtcc.dll libtcc.dll

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
	@. build/vsdevcmd-env.sh && \
	mkdir -p build/win-msvc && \
	rsp_file=`mktemp` && \
	rsp_win=`cygpath -w "$$rsp_file"` && \
	out_win=`cygpath -w "$(CURDIR)/$@"` && \
	obj_dir_win=`cygpath -w "$(CURDIR)/build/win-msvc"` && \
	libtcc_win=`cygpath -w "$(CURDIR)/lib/tinycc/libtcc.lib"` && \
	for source in $(MSVC_SOURCES); do printf '"%s"\n' "`cygpath -w "$(CURDIR)/$$source"`" >> "$$rsp_file"; done && \
	printf '"%s"\n' "$$libtcc_win" >> "$$rsp_file" && \
	MSYS2_ARG_CONV_EXCL='*' cl.exe $(msvc_cflags) /Fo"$$obj_dir_win\\\\" /Fe"$$out_win" @"$$rsp_win" $(msvc_ldflags) && \
	rm -f "$$rsp_file"

cjit-ar.exe: tinycc-msvc
	@. build/vsdevcmd-env.sh && \
	mkdir -p build/win-msvc && \
	src_win=`cygpath -w "$(CURDIR)/src/cjit-ar.c"` && \
	out_win=`cygpath -w "$(CURDIR)/$@"` && \
	obj_win=`cygpath -w "$(CURDIR)/build/win-msvc/cjit-ar.obj"` && \
	libtcc_win=`cygpath -w "$(CURDIR)/lib/tinycc/libtcc.lib"` && \
	MSYS2_ARG_CONV_EXCL='*' cl.exe $(msvc_cflags) /DCJIT_AR_MAIN /Fo"$$obj_win" /Fe"$$out_win" "$$src_win" "$$libtcc_win" $(msvc_ldflags)

win-msvc: cjit.exe cjit-ar.exe
	@rm -f .build_done*
	date | tee .build_done_win_msvc
