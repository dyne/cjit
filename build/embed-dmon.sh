#!/usr/bin/env bash

includes=lib/dmon
dst=src/embed-dmon.c

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

([ "$1" = "code" ]) && {
  externs=`mktemp`
  calls=`mktemp`
}

echo "// ${includes}" > $dst
ipath=`echo ${includes} | sed 's/\//_/g; s/\./_/g'`
for l in $(ls ${includes}); do
	[ -z "$l" ] && continue
	[ "${l:0:1}" = "#" ] && continue

	xxd -i ${includes}/${l} >> $dst
	hname=`echo $l | sed 's/\//_/g; s/\./_/g'`
	([ "$1" = "code" ]) && {
		# print on stderr something handy to paste in code
		echo "extern char *${ipath}_${hname};" >> $externs
		echo "extern unsigned int ${ipath}_${hname}_len;" >> $externs
		echo "if(!write_to_file(tmpdir,\"${l}\",(char*)&${ipath}_${hname},${ipath}_${hname}_len)) goto endgame;" >> $calls
	}
done
# must add const in linux or darwin
# sed inplace is not portable
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i'' -e 's/unsigned char/const char/' $dst
    sed -i'' -e 's/unsigned int/const unsigned int/' $dst
else
    sed -i -e 's/unsigned char/const char/' $dst
    sed -i -e 's/unsigned int/const unsigned int/' $dst
fi
([ "$1" = "code" ]) && {
  >&2 echo "Externs to declare:"
  cat $externs
  >&2 echo
  >&2 echo "Calls:"
  cat $calls
  rm -f $externs $calls
}

exit 0
