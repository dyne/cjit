include build/init.mk

cc := clang


cflags := -Wall -O3
cflags += -Isrc -Ilib/tinycc
# cflags += -DLIBC_GNU -D_GNU_SOURCE

ldadd+=lib/tinycc/libtcc.a

all: deps cjit.command

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

deps: lib/tinycc/libtcc.a src/embed-libtcc1.c

# tinycc_config += --with-libgcc
# tinycc_config += --enable-static --extra-cflags=-static --extra-ldflags=-static



include build/deps.mk
