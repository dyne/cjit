#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
baseline="$temporary/baseline"
report="$temporary/report"
printf 'maintained_percent=50.0\ntolerance_percent=0.0\nsrc/main.c=50.00\n' > "$baseline"
printf "File 'src/main.c'\nLines executed:50.00%% of 10\nFile 'lib/tinycc/tcc.c'\nLines executed:0.00%% of 9999\n" > "$report"
./test/coverage_ratchet.sh "$report" "$baseline"
printf "File 'src/main.c'\nLines executed:0.00%% of 10\n" > "$report"
if output=$(./test/coverage_ratchet.sh "$report" "$baseline" 2>&1); then
    printf 'Coverage ratchet accepted uncovered maintained code\n' >&2
    exit 1
fi
grep -Fq 'DELTA file=src/main.c current=0.00 baseline=50.00 delta=-50.00' <<<"$output"
printf 'maintained_percent=0.0\ntolerance_percent=100.0\nsrc/a.c=50.00\nsrc/z.c=50.00\n' > "$baseline"
printf "File 'src/z.c'\nLines executed:40.00%% of 10\nFile 'src/a.c'\nLines executed:40.00%% of 10\n" > "$report"
output=$(./test/coverage_ratchet.sh "$report" "$baseline")
first=$(grep '^DELTA ' <<<"$output" | sed -n '1p')
second=$(grep '^DELTA ' <<<"$output" | sed -n '2p')
[[ "$first" == 'DELTA file=src/a.c current=40.00 baseline=50.00 delta=-10.00' ]]
[[ "$second" == 'DELTA file=src/z.c current=40.00 baseline=50.00 delta=-10.00' ]]
