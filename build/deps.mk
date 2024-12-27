.c.o:
	$(cc) \
	$(cflags) \
	-c $< -o $@ \
	-DVERSION=\"${VERSION}\" \
	-DCURRENT_YEAR=\"${CURRENT_YEAR}\"

lib/tinycc/libtcc.a lib/tinycc/libtcc1.a:
	cd lib/tinycc \
		&& bash ./configure ${tinycc_config} ${extra_tinycc_config}
	${MAKE} -C lib/tinycc libtcc.a
	${MAKE} -C lib/tinycc libtcc1.a

clean:
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean

# UNUSED EXPERIMENTS:
lib/glfw/src/libglfw3.a:
	cd lib/glfw \
		&& cmake .
			-DGLFW_BUILD_EXAMPLES=NO \
			-DGLFW_BUILD_TESTS=NO \
			-DGLFW_BUILD_DOCS=NO
	${MAKE} -C lib/glfw
	cp lib/glfw/deps/linmath.h     lib/glfw/include/GLFW/
	cp lib/glfw/deps/tinycthread.h lib/glfw/include/GLFW/
	cp lib/glfw/deps/nuklear_glfw* lib/glfw/include/GLFW/
	cp -r lib/glfw/deps/glad       lib/glfw/include/GLFW/

lib/glew/lib/libGLEW.a:
	${MAKE} -C lib/glew/auto
	${MAKE} -C lib/glew

# ${MAKE} -C lib/glfw clean
# ${MAKE} -C lib/glew/auto clean
# ${MAKE} -C lib/glew clean
