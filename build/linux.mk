include build/init.mk

GENERATED_POSIX_FILES := src/assets.c src/assets.h \
	src/embed_libtcc1.a.c src/embed_include.c

cflags += -DLIBC_GNU
cflags += -DKILO_SUPPORTED
cflags += -DCJIT_BUILD_LINUX

all: embed-posix cjit cjit-ar

tinycc_config += --with-libgcc
$(info Ignore any error about sestatus below)
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

ifdef SELFHOST
	cflags += -DSELFHOST
endif

cjit: embed-posix ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

${GENERATED_POSIX_FILES}: embed-posix

cjit-ar: cflags += -DCJIT_AR_MAIN
cjit-ar:
	$(cc) $(cflags) -o $@ src/cjit-ar.c ${ldflags} lib/tinycc/libtcc.a

include build/deps.mk
