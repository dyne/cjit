include build/init.mk

cc := gcc

cflags += -DLIBC_GNU -D_GNU_SOURCE -DREPL_SUPPORTED

SOURCES += src/kilo.o

ifdef ASAN
	cflags := -Og -ggdb -DDEBUG=1 -fno-omit-frame-pointer -fsanitize=address
	ldflags := -fsanitize=address -static-libasan
#	tinycc_config += --extra-ldflags="${ldflags}"
endif

tinycc_config += --with-libgcc

all: lib/tinycc/libtcc.a cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
