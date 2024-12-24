#!/bin/bash
set -e
set -x

# this script is designed to run on GNU/Linux
# it will generate a cjit-demo archive usable on all platforms

rm -rf cjit-demo
mkdir -p cjit-demo
cp -ra examples cjit-demo/

mkdir -p cjit-demo/SDL2

# SDL2
ver=2.30.10
url="https://github.com/libsdl-org/SDL/releases/download/release-${ver}"
[ -r SDL2-${ver}-win32-x64.zip ] ||
	wget -q ${url}/SDL2-${ver}-win32-x64.zip
[ -r cjit-demo/SDL2.dll ] ||
	unzip -q -d cjit-demo SDL2-${ver}-win32-x64.zip SDL2.dll
[ -r SDL2-${ver}.zip ] ||
	wget -q ${url}/SDL2-${ver}.zip
[ -r cjit-demo/SDL2/SDL.h ] || {
	unzip -q -d /tmp SDL2-${ver}.zip SDL2-${ver}/include/*
	mv /tmp/SDL2-${ver}/include cjit-demo/SDL2
	rm -rf /tmp/SDL2-${ver}
}

# SDL2_image
ver=2.8.3
url="https://github.com/libsdl-org/SDL_image/releases/download/release-${ver}"
[ -r SDL2_image-${ver}-win32-x64.zip ] ||
	wget -q ${url}/SDL2_image-${ver}-win32-x64.zip
[ -r cjit-demo/SDL2_image.dll ] ||
	unzip -q -d cjit-demo SDL2_image-${ver}-win32-x64.zip SDL2_image.dll
[ -r SDL2_image-${ver}.zip ] ||
	wget -q ${url}/SDL2_image-${ver}.zip
[ -r cjit-demo/SDL2/SDL_image.h ] || {
	unzip -q -d /tmp SDL2_image-${ver}.zip SDL2_image-${ver}/include/SDL_image.h
	mv /tmp/SDL2_image-${ver}/include/SDL_image.h cjit-demo/SDL2/
	rm -rf /tmp/SDL2_image-${ver}
}

# SDL2_ttf
ver=2.22.0
url="https://github.com/libsdl-org/SDL_ttf/releases/download/release-${ver}"
[ -r SDL2_ttf-${ver}-win32-x64.zip ] ||
	wget -q ${url}/SDL2_ttf-${ver}-win32-x64.zip
[ -r cjit-demo/SDL2_ttf.dll ] ||
	unzip -q -d cjit-demo SDL2_ttf-${ver}-win32-x64.zip SDL2_ttf.dll
[ -r SDL2_ttf-${ver}.zip ] ||
	wget -q ${url}/SDL2_ttf-${ver}.zip
[ -r cjit-demo/SDL2/SDL_ttf.h ] || {
	unzip -q -d /tmp SDL2_ttf-${ver}.zip SDL2_ttf-${ver}/SDL_ttf.h
	mv /tmp/SDL2_ttf-${ver}/SDL_ttf.h cjit-demo/SDL2/
	rm -rf /tmp/SDL2_ttf-${ver}
}

ver=2.2.0
url="https://github.com/nigels-com/glew/releases/download/glew-${ver}/glew-${ver}-win32.zip"
[ -r glew-${ver}-win32.zip ] || {
	wget -q ${url}
}
[ -r cjit-demo/glew32.dll ] || {
	unzip -q -d /tmp glew-${ver}-win32.zip
	mv /tmp/glew-${ver}/bin/Release/x64/glew32.dll cjit-demo/
	mv /tmp/glew-${ver}/include/GL cjit-demo/
	rm -rf /tmp/glew-${ver}
}

tar  --format ustar -czf cjit-demo.tar.gz cjit-demo
rm -rf cjit-demo
