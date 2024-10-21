include build/init.mk

cc := clang


cflags := -Wall -O3
cflags += -Isrc -Ilib/tinycc
# cflags += -DLIBC_GNU -D_GNU_SOURCE

ldadd += lib/tinycc/libtcc.a

all: deps cjit.command

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

deps: lib/tinycc/libtcc.a src/embed-libtcc1.c
	sed -i'' -e 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	sed -i'' -e 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c

# src/embed-headers.c

# tinycc_config += --with-libgcc
# tinycc_config += --enable-static --extra-cflags=-static --extra-ldflags=-static



include build/deps.mk
