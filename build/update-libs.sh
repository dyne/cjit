#!/bin/bash

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

# win32ports
odir="lib/win32ports"
fetch unistd.h  https://raw.githubusercontent.com/win32ports/unistd_h/refs/heads/master/unistd.h
fetch strings.h https://raw.githubusercontent.com/win32ports/strings_h/refs/heads/master/strings.h
fetch dirent.h  https://raw.githubusercontent.com/win32ports/dirent_h/refs/heads/master/dirent.h
odir="lib/win32ports/sys"
fetch time.h https://raw.githubusercontent.com/win32ports/sys_time_h/refs/heads/master/sys/time.h
fetch wait.h https://raw.githubusercontent.com/win32ports/sys_wait_h/refs/heads/master/sys/wait.h


[ "$1" = "stb" ] && {
# std headers
	if [ -d stb ]; then cd stb && git pull --rebase; cd -
	else git clone https://github.com/nothings/stb.git
	fi
	mkdir -p lib/stb
	cp stb/*.h lib/stb/
}