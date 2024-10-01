# CJIT GNUmakefile build initialization
# BRANCH := $(shell git symbolic-ref HEAD | cut -d/ -f3-)
# COMMIT := $(shell git rev-parse --short HEAD)
VERSION := $(shell git describe --tags | cut -d- -f1)
CURRENT_YEAR := $(shell date +%Y)

CC ?= gcc
cc := ${CC}

ifdef CCACHE
	cc := ccache ${cc}
endif

CFLAGS ?= -Og -ggdb -DDEBUG=1 -Wall -Wextra

ifdef RELEASE
	CFLAGS := -O2 -fomit-frame-pointer
endif

cflags := ${CFLAGS} -Isrc -Ilib/tinycc

cflags_stack_protect := -fstack-protector-all -D_FORTIFY_SOURCE=2 -fno-strict-overflow

SOURCES := src/io.o src/file.o src/cflag.o src/cjit.o \
 src/embed-libtcc1.o src/embed-headers.o

ldadd := lib/tinycc/libtcc.a

embed_libtcc1 := lib/tinycc/libtcc1.a

tinycc_config ?= --cc=${cc} --extra-cflags="${cflags}"
ifdef DEBUG
	tinycc_config += --debug
endif
