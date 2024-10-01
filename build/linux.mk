include build/init.mk

cc := gcc
cflags += -DLIBC_GNU -D_GNU_SOURCE -DREPL_SUPPORTED
ldadd+=lib/tinycc/libtcc.a

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldadd}

deps: lib/tinycc/libtcc.a

tinycc_config += --with-libgcc

include build/deps.mk
