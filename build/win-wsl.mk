# Cross build using mingw32 inside a WSL virtual machine hosted on Windows

# This target works around some limitations of the tinycc toolchain by
# leveraging the possibility to run c2str.exe at build time

include build/init.mk

cc := x86_64-w64-mingw32-gcc
ar := x86_64-w64-mingw32-ar
cflags += -DLIBC_MINGW32

ldadd += -lrpcrt4 -lshlwapi

ldflags += -static-libgcc

# embed_libtcc1 := lib/tinycc/x86_64-win32-libtcc1.a

all: deps cjit.exe

cjit.exe: ${SOURCES}
	./build/stamp-exe.sh
	$(cc) $(cflags) -o $@ $(SOURCES) cjit.res ${ldflags} ${ldadd}

#	upx-ucl -q $@
# somehow upx compression helps the binary to function: it
# accidentally fixes some headers that are malformed out of mingw

tinycc_config += --targetos=WIN32 --config-backtrace=no --enable-cross
tinycc_config += --ar=${ar}

deps:
	cd lib/tinycc && ./configure ${tinycc_config}
	$(MAKE) -C lib/tinycc cross-x86_64-win32
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a
	mv lib/tinycc/x86_64-win32-libtcc1.a lib/tinycc/libtcc1.a

include build/deps.mk
