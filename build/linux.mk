include build/init.mk

cc := gcc
cflags += -DLIBC_GNU -D_GNU_SOURCE -DREPL_SUPPORTED
ldadd+=lib/tinycc/libtcc.a

ifdef ASAN
	cflags += -O0 -ggdb -DDEBUG=1 -fno-omit-frame-pointer -fsanitize=address
	ldflags += -fsanitize=address -static-libasan
#	tinycc_config += --extra-ldflags="${ldflags}"
endif

all: deps cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

deps: lib/tinycc/libtcc.a

tinycc_config += --with-libgcc

include build/deps.mk
