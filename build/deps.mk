.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

lib/tinycc/libtcc.a lib/tinycc/libtcc1.a:
	cd lib/tinycc \
		&& ./configure ${tinycc_config} ${extra_tinycc_config}
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean
