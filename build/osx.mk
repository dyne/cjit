include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX

SOURCES += src/kilo.o

all: embed-posix cjit.command

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
