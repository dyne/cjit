#!/bin/bash

includes=${1:-lib/tinycc/include}
dst=${2:-src/embed-headers.c}

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

echo "// ${includes}" > $dst
ipath=`echo $includes | sed 's/\//_/g; s/\./_/g'`
while IFS= read -r line; do
  [ "$line"      = ""  ] && continue
  [ "${line:0:1}" = "#" ] && continue
  xxd -i ${includes}/${line} >> $dst
  hname=`echo $line | sed 's/\//_/g; s/\./_/g'`
  >&2 echo "extern char *${ipath}_${hname};"
  >&2 echo "extern unsigned int ${ipath}_${hname}_len;"
done <<EOF
`ls ${includes}`
EOF

sed -i 's/unsigned char/const char/' $dst
sed -i 's/unsigned int/const unsigned int/' $dst
