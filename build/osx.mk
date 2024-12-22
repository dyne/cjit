include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_misc.o \
	src/embed_stb.o

all: embed cjit.command

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

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
