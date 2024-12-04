
.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

src/embed-musl-libc.c:
	bash build/embed-musl-libc.sh
	sed -i 's/unsigned char _lib_x86_64_linux_musl_libc_so/const unsigned char musl_libc/' src/embed-musl-libc.c
	sed -i 's/unsigned int _lib_x86_64_linux_musl_libc_so_len/const unsigned int musl_libc_len/' src/embed-musl-libc.c

src/embed-libtcc1.c:
	$(info Embedding libtcc1: lib/tinycc/libtcc1.a)
	bash build/embed-libtcc1.sh lib/tinycc/libtcc1.a
	@sed -i 's/unsigned char lib_tinycc_libtcc1_a/const unsigned char libtcc1/' src/embed-libtcc1.c
	@sed -i 's/unsigned int lib_tinycc_libtcc1_a_len/const unsigned int libtcc1_len/' src/embed-libtcc1.c

src/embed-headers.c:
	$(info Embedding tinycc headers)
	bash build/embed-headers.sh
	sed -i 's/unsigned char/const char/' src/embed-headers.c
	sed -i 's/unsigned int/const unsigned int/' src/embed-headers.c

lib/tinycc/libtcc.a:
	cd lib/tinycc && ./configure ${tinycc_config} ${extra_tinycc_config}
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean
