#!/bin/bash
set -e
#set -x

# this script is designed to run on GNU/Linux
# it will generate a cjit-demo archive usable on all platforms
function fetch() {
	[ -z $odir ] && {
		>&2 echo "Script error: \$odir not set"
		exit 1
	}
	out="$1"
	url="$2"
	mkdir -p ${odir}
	if [ -r ${odir}/${out} ]; then
			>&2 echo "Found   : ${odir}/${out}"
	else
		>&2 echo "Download: ${odir}/${out}"
		curl -sL --output ${odir}/${out} ${url}
	fi
}

rm -rf cjit-demo
mkdir -p cjit-demo/include
cp -ra examples cjit-demo/

odir=dl

fetch dmon.h    https://raw.githubusercontent.com/septag/dmon/master/dmon.h
cp dl/dmon.h cjit-demo/include/
fetch nuklear.h https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/master/nuklear.h
cp dl/nuklear.h cjit-demo/include
fetch miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h
cp dl/miniaudio.h cjit-demo/include
fetch termbox2.h https://raw.githubusercontent.com/termbox/termbox2/refs/heads/master/termbox2.h
cp dl/termbox2.h cjit-demo/include

mkdir -p cjit-demo/include/SDL2

# SDL2
ver=2.30.10
file=SDL2-${ver}-win32-x64.zip
url="https://github.com/libsdl-org/SDL/releases/download/release-${ver}"
fetch ${file} ${url}/${file}
[ -r cjit-demo/SDL2.dll ] || unzip -q -d cjit-demo ${file} SDL2.dll
file=SDL2-${ver}
fetch ${file}.zip ${url}/${file}.zip
[ -r cjit-demo/include/SDL2/SDL.h ] || {
	unzip -q -d /tmp ${file}.zip SDL2-${ver}/include/*
	mv /tmp/${file}/include/* cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

# SDL2_image
ver=2.8.3
file=SDL2_image-${ver}-win32-x64.zip
url="https://github.com/libsdl-org/SDL_image/releases/download/release-${ver}"
fetch ${file} ${url}/${file}
[ -r cjit-demo/SDL2_image.dll ] ||
	unzip -q -d cjit-demo ${file} SDL2_image.dll
file=SDL2_image-${ver}
fetch ${file}.zip ${url}/${file}.zip
[ -r cjit-demo/include/SDL2/SDL_image.h ] || {
	unzip -q -d /tmp ${file}.zip ${file}/include/SDL_image.h
	mv /tmp/${file}/include/SDL_image.h cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

# SDL2_ttf
ver=2.22.0
url="https://github.com/libsdl-org/SDL_ttf/releases/download/release-${ver}"
file=SDL2_ttf-${ver}-win32-x64.zip
fetch ${file} ${url}/${file}
[ -r cjit-demo/SDL2_ttf.dll ] ||
	unzip -q -d cjit-demo ${file} SDL2_ttf.dll
file=SDL2_ttf-${ver}
fetch ${file}.zip ${url}/${file}.zip
[ -r cjit-demo/include/SDL2/SDL_ttf.h ] || {
	unzip -q -d /tmp ${file} ${file}/SDL_ttf.h
	mv /tmp/${file}/SDL_ttf.h cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

ver=2.2.0
url="https://github.com/nigels-com/glew/releases/download/glew-${ver}"
file="glew-${ver}-win32.zip"
fetch ${file} ${url}/${file}
[ -r cjit-demo/glew32.dll ] || {
	unzip -q -d /tmp ${file}
	mv /tmp/glew-${ver}/bin/Release/x64/glew32.dll cjit-demo/
	mv /tmp/glew-${ver}/include/GL cjit-demo/include/
	rm -rf /tmp/glew-${ver}
}

tar  --format ustar -cf cjit-demo.tar cjit-demo
gzip -f -9 cjit-demo.tar
rm -rf cjit-demo
exit 0
