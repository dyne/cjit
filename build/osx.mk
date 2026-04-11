include build/init.mk

GENERATED_POSIX_FILES := src/assets.c src/assets.h \
	src/embed_libtcc1.a.c src/embed_include.c

cc := clang
cflags += -DCJIT_BUILD_OSX
extra_tinycc_config += --config-codesign=no --config-backtrace=no

all: embed-posix cjit.command cjit-ar.command


cjit.command: embed-posix ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

${GENERATED_POSIX_FILES}: embed-posix

cjit-ar.command: cflags += -DCJIT_AR_MAIN
cjit-ar.command:
	$(cc) $(cflags) -o $@ src/cjit-ar.c ${ldflags} lib/tinycc/libtcc.a

include build/deps.mk
