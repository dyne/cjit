CC := musl-gcc

include build/init.mk

cflags := -Wall -static -O2 ${cflags_stack_protect}
cflags += -Isrc -Ilib/tinycc -DLIBC_MUSL -nostdlib -DREPL_SUPPORTED

ldadd := lib/tinycc/libtcc.a /usr/lib/x86_64-linux-musl/crt1.o /usr/lib/x86_64-linux-musl/libc.a

SOURCES += src/embed-musl-libc.o

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

deps: lib/tinycc/libtcc.a src/embed-musl-libc.c src/embed-libtcc1.c
	sed -i 's/unsigned char _lib_x86_64_linux_musl_libc_so/const unsigned char musl_libc/' src/embed-musl-libc.c
	sed -i 's/unsigned int _lib_x86_64_linux_musl_libc_so_len/const unsigned int musl_libc_len/' src/embed-musl-libc.c
	sed -i 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	sed -i 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c

tinycc_config += --config-musl --enable-static
tynycc_config += --extra-cflags=-static --extra-ldflags=-static
# check:
# 	./cjit test/hello.c

include build/deps.mk
