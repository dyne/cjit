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
cflags += -DLIBC_MINGW32 -DCJIT_BUILD_WIN
ldflags := -static-libgcc
ldadd := lib/tinycc/libtcc.a -lshlwapi

SOURCES += src/win-compat.o  \
	src/embed_libtcc1.a.o     \
	src/embed_include.o \
	src/embed_contrib_headers.o \
	src/embed_tinycc_win32.o

all: embed cjit.exe

embed: lib/tinycc/libtcc1.a
	$(info Generating embeddings)
	bash build/init-embeddings.sh
	bash build/embed-path.sh lib/tinycc/libtcc1.a
	bash build/embed-path.sh lib/tinycc/include
	bash build/embed-path.sh lib/tinycc/win32/include tinycc_win32
	bash build/embed-path.sh lib/contrib_headers
	@echo                 >> src/embedded.c
	@echo "return(true);" >> src/embedded.c
	@echo "}"             >> src/embedded.c
	@echo          >> src/embedded.h
	@echo "#endif" >> src/embedded.h

cjit.exe: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

# libtcc is built by CI

.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"
