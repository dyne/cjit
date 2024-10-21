CC := musl-gcc

include build/init.mk

cflags := -Wall -static -O2 ${cflags_stack_protect}
cflags += -Isrc -Ilib/tinycc -DLIBC_MUSL -nostdlib
cflags += -DREPL_SUPPORTED

ldadd := lib/tinycc/libtcc.a /usr/lib/x86_64-linux-musl/crt1.o /usr/lib/x86_64-linux-musl/libc.a

SOURCES += src/embed-musl-libc.o src/kilo.o

tinycc_config += --config-musl --enable-static
tynycc_config += --extra-cflags=-static --extra-ldflags=-static

all: lib/tinycc/libtcc.a cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

include build/deps.mk
