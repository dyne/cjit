# Native windows build using mingw
#
# tinycc is built separately by caller:
#   cd .\lib\tinycc
#   bash configure --targetos=WIN32 --config-backtrace=no
#   make libtcc.a libtcc1.a
#
# then run this directly:
#   make -f build/win-mingw.mk

# use only SOURCES from init
include build/init.mk

GENERATED_WIN_FILES := src/assets.c src/assets.h \
	src/embed_libtcc1.a.c src/embed_include.c \
	src/embed_tinycc_win32.c src/embed_win32ports.c

SHELL := C:\Program Files\Git\bin\bash.exe

# redefine compilation flags
CC = gcc
cc := $(CC)
cflags := -O2 -fomit-frame-pointer -Isrc -Ilib/tinycc -Ilib/muntarfs
cflags += -DCJIT_BUILD_WIN
ldflags := -static-libgcc
ldadd := lib/tinycc/libtcc.a -lshlwapi

tinycc_config += --targetos=WIN32 --config-backtrace=no

SOURCES := $(filter-out src/adapters/platform/library_resolver_posix.o,$(SOURCES))
SOURCES += src/win-compat.o  \
	src/embed_tinycc_win32.o \
	src/embed_win32ports.o

all: embed-win cjit.exe cjit-ar.exe

cjit.exe: embed-win ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

${GENERATED_WIN_FILES}: embed-win

cjit-ar.exe: cflags += -DCJIT_AR_MAIN
cjit-ar.exe:
	$(cc) $(cflags) -o $@ src/cjit-ar.c ${ldflags} lib/tinycc/libtcc.a

# libtcc is built by CI

include build/deps.mk
