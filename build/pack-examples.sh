#!/bin/bash
set -e
set -x

# this script is designed to run on GNU/Linux
# it will generate a cjit-demo archive usable on all platforms
function fetch() {
	[ -z $odir ] && {
		>&2 echo "Script error: \$odir not set"
		exit 1
	}
	local out="$1"
	local url="$2"
	local REPO_OWNER="$3"
	local REPO_NAME="$4"
	local TAG="$5"
	local FILE_NAME="$out"
	if [ -r ${odir}/${out} ]; then
			>&2 echo "Found   : ${odir}/${out}"
	else
		>&2 echo "Download: ${odir}/${out}"
		if [ "$GITHUB_ACTIONS" == "true" ]; then
			API_URL="https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/tags/$TAG"
			DOWNLOAD_URL=$(curl -s -H "Authorization: token $GITHUB_TOKEN" $API_URL | jq -r ".assets[] | select(.name==\"$FILE_NAME\") | .browser_download_url")
			curl -L -H "Authorization: token $GITHUB_TOKEN" \
				 --output ${odir}/$FILE_NAME $DOWNLOAD_URL
		else
			curl -sL --output ${odir}/${out} ${url}
		fi
	fi
}

rm -rf cjit-demo
mkdir -p cjit-demo/include
cp -ra examples cjit-demo/

odir=dl
mkdir -p ${odir}

[ -r ${odir}/dmon.h ] ||
	curl -L --output ${odir}/dmon.h https://raw.githubusercontent.com/septag/dmon/master/dmon.h
cp dl/dmon.h cjit-demo/include/
[ -r ${odir}/nuklear.h ] ||
    curl -L --output ${odir}/nuklear.h https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/master/nuklear.h
cp dl/nuklear.h cjit-demo/include
[ -r ${odir}/miniaudio.h ] ||
	curl -L --output ${odir}/miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h
cp dl/miniaudio.h cjit-demo/include
[ -r ${odir}/termbox2.h ] ||
	curl -L --output ${odir}/termbox2.h https://raw.githubusercontent.com/termbox/termbox2/refs/heads/master/termbox2.h
cp dl/termbox2.h cjit-demo/include

mkdir -p cjit-demo/include/SDL2

# SDL2
org=libsdl-org


proj=SDL
ver=2.30.10
file=SDL2-${ver}-win32-x64.zip
url="https://github.com/${org}/${proj}/releases/download/release-${ver}"
fetch ${file} ${url}/${file} ${org} ${proj} release-${ver}
[ -r cjit-demo/SDL2.dll ] || unzip -q -d cjit-demo ${odir}/${file} SDL2.dll
file=SDL2-${ver}
fetch ${file}.zip ${url}/${file}.zip ${org} ${proj} release-${ver}
[ -r cjit-demo/include/SDL2/SDL.h ] || {
	unzip -q -d /tmp ${odir}/${file}.zip SDL2-${ver}/include/*
	mv /tmp/${file}/include/* cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

# SDL2_image
proj=SDL_image
ver=2.8.3
file=SDL2_image-${ver}-win32-x64.zip
url="https://github.com/${org}/${proj}/releases/download/release-${ver}"
fetch ${file} ${url}/${file} ${org} ${proj} release-${ver}
[ -r cjit-demo/SDL2_image.dll ] ||
	unzip -q -d cjit-demo ${odir}/${file} SDL2_image.dll
file=SDL2_image-${ver}
fetch ${file}.zip ${url}/${file}.zip ${org} ${proj} release-${ver}
[ -r cjit-demo/include/SDL2/SDL_image.h ] || {
	unzip -q -d /tmp ${odir}/${file}.zip ${file}/include/SDL_image.h
	mv /tmp/${file}/include/SDL_image.h cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

# SDL2_ttf
proj=SDL_ttf
ver=2.22.0
url="https://github.com/${org}/${proj}/releases/download/release-${ver}"
file=SDL2_ttf-${ver}-win32-x64.zip
fetch ${file} ${url}/${file} ${org} ${proj} release-${ver}
[ -r cjit-demo/SDL2_ttf.dll ] ||
	unzip -q -d cjit-demo ${odir}/${file} SDL2_ttf.dll
file=SDL2_ttf-${ver}
fetch ${file}.zip ${url}/${file}.zip ${org} ${proj} release-${ver}
[ -r cjit-demo/include/SDL2/SDL_ttf.h ] || {
	unzip -q -d /tmp ${odir}/${file} ${file}/SDL_ttf.h
	mv /tmp/${file}/SDL_ttf.h cjit-demo/include/SDL2/
	rm -rf /tmp/${file}
}

# raylib
proj=raylib
ver=5.5
org=raysan5
file=${proj}-${ver}_win64_msvc16
url="https://github.com/${org}/${proj}/releases/download/${ver}"
fetch ${file}.zip ${url}/${file}.zip ${org} ${proj} ${ver}
[ -r cjit-demo/raylib.dll ] || {
	unzip -q -d /tmp ${odir}/${file} ${file}/lib/raylib.dll
	mv /tmp/${file}/lib/raylib.dll cjit-demo/raylib.dll
	rm -rf /tmp/${file}
}
[ -r cjit-demo/include/raylib.h ] || {
	unzip -q -d /tmp ${odir}/${file} ${file}/include/*
	mv /tmp/${file}/include/* cjit-demo/include/
	rm -rf /tmp/${file}
}
[ -r cjit-demo/libraylib.so ] || {
	file=${proj}-${ver}_linux_amd64
	fetch ${file}.tar.gz ${url}/${file}.tar.gz ${org} ${proj} ${ver}
	tar -C /tmp -xvf ${odir}/${file}.tar.gz
	mv /tmp/${file}/lib/libraylib.so.${ver}.0 cjit-demo/libraylib.so
	rm -rf /tmp/${file}
}

proj=glew
ver=2.2.0
org=nigels-com
url="https://github.com/${org}/${proj}/releases/download/glew-${ver}"
file="glew-${ver}-win32.zip"
fetch ${file} ${url}/${file} ${org} ${proj} glew-${ver}
[ -r cjit-demo/glew32.dll ] || {
	unzip -q -d /tmp ${odir}/${file}
	mv /tmp/glew-${ver}/bin/Release/x64/glew32.dll cjit-demo/
	mv /tmp/glew-${ver}/include/GL cjit-demo/include/
	rm -rf /tmp/glew-${ver}
}

tar  --format ustar -cf cjit-demo.tar cjit-demo
gzip -f -9 cjit-demo.tar
# rm -rf cjit-demo
exit 0
