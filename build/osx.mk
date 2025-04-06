include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX
extra_tinycc_config += --config-codesign=no

all: embed-posix cjit.command cjit-ar.command


cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

cjit-ar.command: src/cjit-ar.o
	$(cc) $(cflags) -o $@ src/cjit-ar.o ${ldflags} lib/tinycc/libtcc.a

include build/deps.mk
