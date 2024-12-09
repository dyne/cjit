# Cross build using mingw32 inside a WSL virtual machine hosted on Windows

# This target works around some limitations of the tinycc toolchain by
# leveraging the possibility to run c2str.exe at build time

include build/init.mk

cc := x86_64-w64-mingw32-gcc
ar := x86_64-w64-mingw32-ar
cflags += -DLIBC_MINGW32 -DCJIT_BUILD_WIN

ldadd += -lrpcrt4 -lshlwapi

ldflags += -static-libgcc

tinycc_config += --targetos=WIN32 --config-backtrace=no --enable-cross
tinycc_config += --ar=${ar}

SOURCES += src/win-compat.o

all: deps cjit.exe

cjit.exe: ${SOURCES}
	./build/stamp-exe.sh
	$(cc) $(cflags) -o $@ $(SOURCES) cjit.res ${ldflags} ${ldadd}

deps:
	@cd lib/tinycc && ./configure ${tinycc_config}
	@$(MAKE) -C lib/tinycc cross-x86_64-win32
	@${MAKE} -C lib/tinycc libtcc.a
	@${MAKE} -C lib/tinycc libtcc1.a
	@mv lib/tinycc/x86_64-win32-libtcc1.a lib/tinycc/libtcc1.a
	@bash build/embed-libtcc1.sh lib/tinycc/libtcc1.a
	@sed -i 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	@sed -i 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c
	@bash build/embed-headers.sh win
	@sed -i 's/unsigned char/const char/' src/embed-headers.c
	@sed -i 's/unsigned int/const unsigned int/' src/embed-headers.c
	@bash build/embed-dmon.sh

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"
