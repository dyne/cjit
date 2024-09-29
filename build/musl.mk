include build/init.mk

cc := musl-gcc
cflags += -static -Os
cflags += -Isrc -Ilib/tinycc -DLIBC_MUSL

ldadd := lib/tinycc/libtcc.a -lc

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

deps: lib/tinycc/libtcc.a src/embed-musl-libc.c src/embed-libtcc1.c

tinycc_config += --config-musl --enable-static
tynycc_config += --extra-cflags=-static --extra-ldflags=-static
# check:
# 	./cjit test/hello.c

include build/deps.mk
