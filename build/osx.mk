include build/init.mk

cc := clang
cflags += -DCJIT_BUILD_OSX

SOURCES += \
	src/kilo.o \
	src/embed_libtcc1.a.o \
	src/embed_include.o \
	src/embed_contrib_headers.o

all: embed cjit.command

embed: lib/tinycc/libtcc1.a
	$(info Generating embeddings)
	bash build/init-embeddings.sh
	bash build/embed-path.sh lib/tinycc/libtcc1.a
	bash build/embed-path.sh lib/tinycc/include
	bash build/embed-path.sh lib/contrib_headers
	@echo "\nreturn(true);\n}\n" >> src/embedded.c
	@echo "\n#endif\n" >> src/embedded.h

cjit.command: ${SOURCES}
	$(cc) $(cflags) -o $@ $(SOURCES) ${ldflags} ${ldadd}

include build/deps.mk
