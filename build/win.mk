include build/init.mk

cc := x86_64-w64-mingw32-gcc
ar := x86_64-w64-mingw32-ar
cflags += -DLIBC_MINGW32 -D_GNU_SOURCE
ldadd += -lrpcrt4 -lshlwapi
ldflags += -static-libgcc

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

deps: lib/tinycc/libtcc.a

tinycc_config += --targetos=WIN32

include build/deps.mk
