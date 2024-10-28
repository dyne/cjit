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

cflags_includes := -Isrc -Ilib/tinycc
cflags_stack_protect := -fstack-protector-all -D_FORTIFY_SOURCE=2 -fno-strict-overflow

ifdef RELEASE
	CFLAGS := -O2 -fomit-frame-pointer ${cflags_stack_protect}
endif

cflags := ${CFLAGS} ${cflags_includes}

SOURCES := src/io.o src/file.o src/cflag.o src/cjit.o \
           src/embed-libtcc1.o src/embed-headers.o \
           src/exec-headers.o

ldadd := lib/tinycc/libtcc.a

tinycc_config ?= --cc=${cc} --extra-cflags="${cflags}" --extra-ldflags="${ldflags}"

ifdef DEBUG
	tinycc_config += --debug
endif
