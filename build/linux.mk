include build/init.mk

cc := gcc

cflags += -DLIBC_GNU -D_GNU_SOURCE
cflags += -DKILO_SUPPORTED
cflags += -DCJIT_BUILD_LINUX

all: embed-posix cjit

tinycc_config += --with-libgcc
ifeq ($(shell sestatus | awk -F': *' '/SELinux status:/ {print $2}'), enabled)
tinycc_config += --with-selinux
endif

ifdef ASAN
	ASAN_FLAGS := -fsanitize=address -fsanitize=leak
	ASAN_FLAGS += -fsanitize=float-divide-by-zero
	ASAN_FLAGS += -fsanitize=float-cast-overflow
	cflags := -g -DDEBUG=1 -Wall -fno-omit-frame-pointer
	cflags += ${ASAN_FLAGS} -DMEM_DEBUG
	cflags += ${cflags_includes} ${cflags_gnu} -DKILO_SUPPORTED
	cflags += -DCJIT_BUILD_LINUX
	ldflags := ${ASAN_FLAGS} -static-libasan
endif

ifdef GDB
	cflags := -Og -ggdb -DDEBUG=1 -fno-omit-frame-pointer
	cflags += ${cflags_includes} ${cflags_gnu} -DKILO_SUPPORTED
	cflags += -DCJIT_BUILD_LINUX
#	tinycc_config += --extra-ldflags="${ldflags}"
endif

cjit: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
