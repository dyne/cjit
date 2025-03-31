#!/bin/bash

[ "$1" = "" ] && {
	>&2 echo "Usage: $0 file version"
	exit 1
}
file="$1"
[ -r ${file} ] || {
	>&2 echo "File not found: $file"
	exit 1
}
version="$2"
[ "$2" = "" ] && {
	>&2 echo "Usage: $0 file version"
	exit 1
}

hash=`sha256sum -bz $file | awk '{print $1}' | tr '[:lower:]' '[:upper:]'`
mkdir -p manifests/d/Dyne/CJIT/${version}

>&2 echo "Rendering manifest: $file $version"
>&2 echo "$hash"

function render() {
	src="$1"
	[ -r build/winpkg-manifest/${src} ] || {
		>&2 echo "Source not found: $src"
		exit 1
	}
	>&2 echo -n "Rendering $src... "
	sed 's/%%VERSION%%/'"${version}"'/; s/%%INSTALLER_HASH_SHA256%%/'"$hash"'/' build/winpkg-manifest/${src} > manifests/d/Dyne/CJIT/${version}/${src}
	>&2 echo "OK"
}

render Dyne.CJIT.installer.yaml
render Dyne.CJIT.locale.en-US.yaml
render Dyne.CJIT.yaml
