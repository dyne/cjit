include build/init.mk

cc := gcc

cflags += -DLIBC_GNU -D_GNU_SOURCE
cflags += -DKILO_SUPPORTED
cflags += -DCJIT_BUILD_LINUX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_contrib_headers.o

all: embed cjit

embed: lib/tinycc/libtcc1.a
	$(info Generating embeddings)
	bash build/init-embeddings.sh
	bash build/embed-path.sh lib/tinycc/libtcc1.a
	bash build/embed-path.sh lib/tinycc/include
	bash build/embed-path.sh lib/contrib_headers
	@echo "\nreturn(true);\n}\n" >> src/embedded.c
	@echo "\n#endif\n" >> src/embedded.h

tinycc_config += --with-libgcc
ifeq ($(shell sestatus | awk -F': *' '/SELinux status:/ {print $2}'), enabled)
tinycc_config += --with-selinux
endif

ifdef ASAN
	cflags := -Og -ggdb -DDEBUG=1 -fno-omit-frame-pointer -fsanitize=address
	cflags += ${cflags_includes} ${cflags_gnu} -DKILO_SUPPORTED
	cflags += -DCJIT_BUILD_LINUX
	ldflags := -fsanitize=address -static-libasan
#	tinycc_config += --extra-ldflags="${ldflags}"
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
