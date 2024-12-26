# CJIT GNUmakefile build initialization
# BRANCH := $(shell git symbolic-ref HEAD | cut -d/ -f3-)
# COMMIT := $(shell git rev-parse --short HEAD)
VERSION := $(shell git describe --tags | cut -d- -f1)
CURRENT_YEAR := $(shell date +%Y)

CC ?= gcc
cc := ${CC}

cflags_includes := -Isrc -Ilib/tinycc
cflags_gnu := -DLIBC_GNU -D_GNU_SOURCE
cflags_stack_protect := -fstack-protector-all -D_FORTIFY_SOURCE=2 -fno-strict-overflow

CFLAGS ?= -O2 -fomit-frame-pointer ${cflags_stack_protect}

cflags := ${CFLAGS} ${cflags_includes}

SOURCES := src/file.o src/cflag.o src/cjit.o \
           src/main.o src/assets.o \
           src/repl.o \
           src/muntar.o src/tinflate.o src/tinfgzip.o \
           src/embed_libtcc1.a.o src/embed_include.o \
           src/embed_misc.o src/embed_stb.o

#src/embed_source.o


ldadd := lib/tinycc/libtcc.a

tinycc_config ?= --cc=${cc} --extra-cflags="${cflags}" --extra-ldflags="${ldflags}"

ifdef DEBUG
	tinycc_config += --debug
endif

embed-posix: lib/tinycc/libtcc1.a
	$(info Embedding assets for POSIX build)
	bash build/init-assets.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh assets/misc
	bash build/embed-asset-path.sh assets/stb
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h

# 	bash build/embed-source.sh

embed-win: lib/tinycc/libtcc.a lib/tinycc/libtcc1.a
	$(info Embedding assets for Windows build)
	bash build/init-assets.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh lib/tinycc/win32/include tinycc_win32
	bash build/embed-asset-path.sh assets/win32ports
	bash build/embed-asset-path.sh assets/misc
	bash build/embed-asset-path.sh assets/stb
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h

embed-musl: lib/tinycc/libtcc1.a
	$(info Generating assets)
	bash build/init-assetss.sh
	bash build/embed-asset-path.sh lib/tinycc/libtcc1.a
	bash build/embed-asset-path.sh lib/tinycc/include
	bash build/embed-asset-path.sh assets/misc
	bash build/embed-asset-path.sh /lib/x86_64-linux-musl/libc.so
	bash build/embed-asset-path.sh assets/stb
	@echo                 >> src/assets.c
	@echo "return(true);" >> src/assets.c
	@echo "}"             >> src/assets.c
	@echo          >> src/assets.h
	@echo "#endif" >> src/assets.h
