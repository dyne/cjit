# Native windows build using mingw
#
# tinycc is built separately by hand:
#   cd .\lib\tinycc
#   bash configure --targetos=WIN32 --config-backtrace=no
#   make libtcc.a libtcc1.a
#
# then run this directly:
#   make -f build/win-native.mk

SHELL := C:\Program Files\Git\bin\bash.exe

cc := gcc

cflags := -O2 -fomit-frame-pointer -Isrc -Ilib/tinycc
cflags += -DLIBC_MINGW32

SOURCES := src/io.o src/file.o src/cflag.o \
	src/cjit.o src/embed-libtcc1.o src/embed-headers.o

ldflags += -static-libgcc

ldadd := lib/tinycc/libtcc.a -lshlwapi

# The libtcc is built by the calling GNUmakefile
# to: lib/tinycc/libtcc.a

cjit.exe: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

src/embed-libtcc1.c:
	$(info Embedding libtcc1)
	sh build/embed-libtcc1.sh lib/tinycc/libtcc1.a
	sed -i 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	sed -i 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c

src/embed-headers.c:
	$(info Embedding tinycc headers)
	bash build/embed-headers.sh win
	sed -i 's/unsigned char/const char/' src/embed-headers.c
	sed -i 's/unsigned int/const unsigned int/' src/embed-headers.c
