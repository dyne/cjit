# Cross build using mingw32 inside a WSL virtual machine hosted on Windows

# This target works around some limitations of the tinycc toolchain by
# leveraging the possibility to run c2str.exe at build time

include build/init.mk

cc := x86_64-w64-mingw32-gcc
ar := x86_64-w64-mingw32-ar

cflags := -O2 -mconsole ${cflags_includes}
cflags += -DCJIT_BUILD_WIN

ldadd += -lrpcrt4 -lshlwapi

ldflags += -static-libgcc

tinycc_config += --targetos=WIN32 --config-backtrace=no --enable-cross
tinycc_config += --ar=${ar}

SOURCES += src/win-compat.o  \
	src/embed_tinycc_win32.o \
	src/embed_win32ports.o

all: cross-win embed-win cjit.exe cjit-ar.exe

cjit.exe: ${SOURCES}
	bash build/stamp-exe.sh
	$(cc) $(cflags) -o $@ $(SOURCES) cjit.res ${ldflags} ${ldadd}

cjit-ar.exe: cflags += -DCJIT_AR_MAIN
cjit-ar.exe: rebuild_cjit-ar
	$(cc) $(cflags) -o $@ src/cjit-ar.o cjit.res ${ldflags} lib/tinycc/libtcc.a
	@rm src/src/cjit-ar.o

cross-win:
	@cd lib/tinycc && ./configure ${tinycc_config}
	@$(MAKE) -C lib/tinycc cross-x86_64-win32
	@${MAKE} -C lib/tinycc libtcc.a
	@${MAKE} -C lib/tinycc libtcc1.a
	@mv lib/tinycc/x86_64-win32-libtcc1.a lib/tinycc/libtcc1.a

include build/deps.mk
