#!/bin/bash

set -e

odir=lib/contrib_headers

function fetch() {
	[ -z $odir ] && {
		>&2 echo "Script error: \$odir not set"
		exit 1
	}
	out="$1"
	url="$2"
	mkdir -p ${odir}
	mkdir -p .${odir}
	if [ -r ${odir}/${out} ];then
		mv ${odir}/${out} .${odir}/${out}
		>&2 echo "Update: ${odir}/${out}"
	else
		>&2 echo "Download: ${odir}/${out}"
	fi
	curl -sL --output ${odir}/${out} ${url}
	[ -r .${odir}/${out} ] || return
    diff ${odir}/${out} .${odir}/${out} > /dev/null || {
		>&2 echo "DIFF ${out}"
		>&2 diff ${odir}/${out} .${odir}/${out}
		>&2 echo "END DIFF"
	}
	rm -rf .${odir}
}

fetch dmon.h    https://raw.githubusercontent.com/septag/dmon/master/dmon.h
fetch nuklear.h https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/master/nuklear.h
fetch miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h
fetch termbox2.h https://raw.githubusercontent.com/termbox/termbox2/refs/heads/master/termbox2.h

# win32ports
odir="lib/win32ports"
fetch unistd.h  https://raw.githubusercontent.com/win32ports/unistd_h/refs/heads/master/unistd.h
fetch strings.h https://raw.githubusercontent.com/win32ports/strings_h/refs/heads/master/strings.h
fetch dirent.h  https://raw.githubusercontent.com/win32ports/dirent_h/refs/heads/master/dirent.h
odir="lib/win32ports/sys"
fetch time.h https://raw.githubusercontent.com/win32ports/sys_time_h/refs/heads/master/sys/time.h
fetch wait.h https://raw.githubusercontent.com/win32ports/sys_wait_h/refs/heads/master/sys/wait.h

function clone() {
	name=$1
	repo=$2
	>&2 echo "update $name headers from $repo"
	git clone --depth 1 $repo /tmp/$name
	mkdir -p lib/$name
}

clone stb https://github.com/nothings/stb.git
cp /tmp/stb/*.h lib/stb
rm -rf /tmp/stb
