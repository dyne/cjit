include build/init.mk

cc := gcc

cflags += -DLIBC_GNU -D_GNU_SOURCE
cflags += -DKILO_SUPPORTED
cflags += -DCJIT_BUILD_LINUX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_misc.o \
	src/embed_stb.o

all: embed cjit

embed: lib/tinycc/libtcc1.a
	$(info Generating embeddings)
	bash build/init-embeddings.sh
	bash build/embed-path.sh lib/tinycc/libtcc1.a
	bash build/embed-path.sh lib/tinycc/include
	bash build/embed-path.sh assets/misc
	bash build/embed-path.sh assets/stb
	@echo                 >> src/embedded.c
	@echo "return(true);" >> src/embedded.c
	@echo "}"             >> src/embedded.c
	@echo          >> src/embedded.h
	@echo "#endif" >> src/embedded.h

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
