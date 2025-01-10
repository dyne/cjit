include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX
extra_tinycc_config += --config-codesign=no

all: embed-posix cjit.command


cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
