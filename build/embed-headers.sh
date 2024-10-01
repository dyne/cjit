#!/bin/bash

includes=lib/tinycc/include
dst=src/embed-headers.c

>&2 echo "To print out code on console run:"
>&2 echo "bash build/embed-headers.sh code"

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

[ "$1" = "code" ] && {
  externs=`mktemp`
  calls=`mktemp`
}
echo "// ${includes}" > $dst
ipath=`echo $includes | sed 's/\//_/g; s/\./_/g'`
while IFS= read -r line; do
  [ "$line"      = ""  ] && continue
  [ "${line:0:1}" = "#" ] && continue
  xxd -i ${includes}/${line} >> $dst
  hname=`echo $line | sed 's/\//_/g; s/\./_/g'`
  [ "$1" = "code" ] && {
    # print on stderr something handy to paste in code
    echo "extern char *${ipath}_${hname};" >> $externs
    echo "extern unsigned int ${ipath}_${hname}_len;" >> $externs
    echo "if(!write_to_file(tmpdir,\"${line}\",(char*)&${ipath}_${hname},${ipath}_${hname}_len)) goto endgame;" >> $calls
  }
done <<EOF
`ls ${includes}`
EOF

sed -i 's/unsigned char/const char/' $dst
sed -i 's/unsigned int/const unsigned int/' $dst

[ "$1" = "code" ] && {
  >&2 echo "Externs to declare:"
  cat $externs
  >&2 echo
  >&2 echo "Calls:"
  cat $calls
  rm -f $externs $calls
}

>&2 echo "Done generating src/embed-headers.c"
