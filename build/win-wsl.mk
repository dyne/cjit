# Cross build using mingw32 inside a WSL virtual machine hosted on Windows

# This target works around some limitations of the tinycc toolchain by
# leveraging the possibility to run c2str.exe at build time

include build/init.mk

cc := x86_64-w64-mingw32-gcc
ar := x86_64-w64-mingw32-ar

cflags := -Og -ggdb ${cflags_includes}
cflags += -DCJIT_BUILD_WIN

ldadd += -lrpcrt4 -lshlwapi

ldflags += -static-libgcc

tinycc_config += --targetos=WIN32 --config-backtrace=no --enable-cross
tinycc_config += --ar=${ar}

SOURCES += src/win-compat.o  \
	src/embed_libtcc1.a.o     \
	src/embed_include.o \
	src/embed_tinycc_win32.o \
	src/embed_win32ports.o \
	src/embed_contrib_headers.o \
	src/embed_stb.o

all: deps embed cjit.exe

embed: lib/tinycc/libtcc1.a
	$(info Generating embeddings)
	bash build/init-embeddings.sh
	bash build/embed-path.sh lib/tinycc/libtcc1.a
	bash build/embed-path.sh lib/tinycc/include
	bash build/embed-path.sh lib/tinycc/win32/include tinycc_win32
	bash build/embed-path.sh lib/win32ports
	bash build/embed-path.sh lib/contrib_headers
	bash build/embed-path.sh lib/stb
	@echo                 >> src/embedded.c
	@echo "return(true);" >> src/embedded.c
	@echo "}"             >> src/embedded.c
	@echo          >> src/embedded.h
	@echo "#endif" >> src/embedded.h

cjit.exe: ${SOURCES}
	./build/stamp-exe.sh
	$(cc) $(cflags) -o $@ $(SOURCES) cjit.res ${ldflags} ${ldadd}

deps:
	@cd lib/tinycc && ./configure ${tinycc_config}
	@$(MAKE) -C lib/tinycc cross-x86_64-win32
	@${MAKE} -C lib/tinycc libtcc.a
	@${MAKE} -C lib/tinycc libtcc1.a
	@mv lib/tinycc/x86_64-win32-libtcc1.a lib/tinycc/libtcc1.a

# @bash build/embed-headers.sh win
# @sed -i 's/unsigned char/const char/' src/embed-headers.c
# @sed -i 's/unsigned int/const unsigned int/' src/embed-headers.c

include build/deps.mk
# .c.o:
# 	$(cc) \
# 	$(cflags) \
# 	-c $< -o $@ \
# 	-DVERSION=\"${VERSION}\" \
# 	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"
