# Native windows build using mingw
#
# tinycc is built separately by caller:
#   cd .\lib\tinycc
#   bash configure --targetos=WIN32 --config-backtrace=no
#   make libtcc.a libtcc1.a
#
# then run this directly:
#   make -f build/win-native.mk

# use only SOURCES from init
include build/init.mk

SHELL := C:\Program Files\Git\bin\bash.exe

# redefine compilation flags
cc := gcc
cflags := -O2 -fomit-frame-pointer -Isrc -Ilib/tinycc
cflags += -DCJIT_BUILD_WIN
ldflags := -static-libgcc
ldadd := lib/tinycc/libtcc.a -lshlwapi

tinycc_config += --targetos=WIN32 --config-backtrace=no

SOURCES += src/win-compat.o  \
	src/embed_tinycc_win32.o \
	src/embed_win32ports.o

all: embed-win cjit.exe cjit-ar.exe

cjit.exe: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

cjit-ar.exe: cflags += -DCJIT_AR_MAIN
cjit-ar.exe:
	$(cc) $(cflags) -o $@ src/cjit-ar.c ${ldflags} lib/tinycc/libtcc.a

# libtcc is built by CI

include build/deps.mk
