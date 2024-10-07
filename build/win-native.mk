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

# embed_libtcc1 := .\lib\tinycc\x86_64-win32-libtcc1.a

cjit.exe: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

src/embed-libtcc1.c:
	$(info Embedding libtcc1: ${embed_libtcc1})
	sh build/embed-libtcc1.sh ${embed_libtcc1}

src/embed-headers.c:
	$(info Embedding tinycc headers)
	bash build/embed-headers.sh win
