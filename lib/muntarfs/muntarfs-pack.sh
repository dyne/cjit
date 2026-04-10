#!/bin/sh
set -eu

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "usage: $0 <source-dir> <output-prefix> [root-name]" >&2
    exit 1
fi

src_dir="$1"
out_prefix="$2"
root_name="${3:-$(basename "$src_dir")}"

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT INT TERM

stage_dir="$tmp_dir/$root_name"
mkdir -p "$stage_dir"
cp -a "$src_dir"/. "$stage_dir"/

tar_file="${out_prefix}.tar"
targz_file="${out_prefix}.tar.gz"
c_file="${out_prefix}.c"

tar --format ustar -cf "$tar_file" -C "$tmp_dir" "$root_name"
gzip -c "$tar_file" > "$targz_file"
xxd -i "$targz_file" > "$c_file"

echo "$tar_file"
echo "$targz_file"
echo "$c_file"
