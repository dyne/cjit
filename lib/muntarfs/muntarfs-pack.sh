#!/bin/sh
#
# muntarfs, part of CJIT
#
# Copyright (C) 2024 Dyne.org foundation
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
