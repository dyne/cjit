include build/init.mk

cc := gcc

cflags += -DLIBC_GNU -D_GNU_SOURCE -DKILO_SUPPORTED

SOURCES += src/kilo.o

ifdef ASAN
	cflags := -Og -ggdb -DDEBUG=1 -fno-omit-frame-pointer -fsanitize=address
	cflags += ${cflags_includes} ${cflags_gnu} -DKILO_SUPPORTED
	ldflags := -fsanitize=address -static-libasan
#	tinycc_config += --extra-ldflags="${ldflags}"
endif

tinycc_config += --with-libgcc

ifeq ($(shell sestatus | awk -F': *' '/SELinux status:/ {print $2}'), enabled)
tinycc_config += --with-selinux
endif

all: lib/tinycc/libtcc.a cjit

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
