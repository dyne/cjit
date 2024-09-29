include build/init.mk

cc := gcc
cflags += -DLIBC_GNU -D_GNU_SOURCE

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

deps: lib/tinycc/libtcc.a

tinycc_config += --with-libgcc

include build/deps.mk
