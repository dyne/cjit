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

SOURCES := src/io.o src/file.o src/cflag.o src/cjit.o \
           src/embed-libtcc1.o src/embed-headers.o \
           src/exec-headers.o src/repl.o src/embed-dmon.o

ldadd := lib/tinycc/libtcc.a

tinycc_config ?= --cc=${cc} --extra-cflags="${cflags}" --extra-ldflags="${ldflags}"

ifdef DEBUG
	tinycc_config += --debug
endif
