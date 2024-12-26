include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_misc.o \
	src/embed_stb.o

all: embed-posix cjit.command

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
