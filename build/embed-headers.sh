#!/bin/bash

includes=lib/tinycc/include
win_includes="lib/tinycc/win32/include/*.h lib/tinycc/win32/include/**/*.h"
dst=src/embed-headers.c

>&2 echo "To print out code on console run:"
>&2 echo "bash build/embed-headers.sh code"

command -v xxd > /dev/null || {
  >&2 echo "Error not found: xxd binary not installed"
  exit 1
}

([ "$1" = "code" ] || [ "$2" = "code" ]) && {
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
  ([ "$1" = "code" ] || [ "$2" = "code" ]) && {
    # print on stderr something handy to paste in code
    echo "extern char *${ipath}_${hname};" >> $externs
    echo "extern unsigned int ${ipath}_${hname}_len;" >> $externs
    echo "if(!write_to_file(tmpdir,\"${l}\",(char*)&${ipath}_${hname},${ipath}_${hname}_len)) goto endgame;" >> $calls
  }
done

if [ "$1" = "win" ]; then
  echo "// ${win_includes}" >> $dst
  ipath=`echo "lib/tinycc/win32/include" | sed 's/\//_/g; s/\./_/g'`
  ([ "$1" = "code" ] || [ "$2" = "code" ]) && {
    echo "#if defined(LIBC_MINGW32)" >> $externs
    echo "#if defined(LIBC_MINGW32)" >> $calls
  }
  for l in $(ls ${win_includes} | sed 's|lib/tinycc/win32/include/||'); do
    [ -z "$l" ] && continue
    [ "${l:0:1}" = "#" ] && continue

    xxd -i "lib/tinycc/win32/include/${l}" >> $dst
    l_win_slash=`echo $l | sed 's|/|\\\\\\\\|g;'`
    hname=`echo $l | sed 's|/|_|g; s|\.|_|g'`
    ([ "$1" = "code" ] || [ "$2" = "code" ]) && {
      # print on stderr something handy to paste in code
      echo "extern char *${ipath}_${hname};" >> $externs
      echo "extern unsigned int ${ipath}_${hname}_len;" >> $externs
      echo "if(!write_to_file(tmpdir,\"${l_win_slash}\",(char*)&${ipath}_${hname},${ipath}_${hname}_len)) goto endgame;" >> $calls
    }
  done
  ([ "$1" = "code" ] || [ "$2" = "code" ]) && {
    echo "#endif" >> $externs
    echo "#endif" >> $calls
  }
fi

sed -i 's/unsigned char/const char/' $dst
sed -i 's/unsigned int/const unsigned int/' $dst

([ "$1" = "code" ] || [ "$2" = "code" ]) && {
  >&2 echo "Externs to declare:"
  cat $externs
  >&2 echo
  >&2 echo "Calls:"
  cat $calls
  rm -f $externs $calls
}

>&2 echo "Done generating src/embed-headers.c"