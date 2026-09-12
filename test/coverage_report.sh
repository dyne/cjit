#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

output=${1:?usage: coverage_report.sh OUTPUT SOURCE...}
shift
tool=${COVERAGE_TOOL:?COVERAGE_TOOL is required}
args=${COVERAGE_TOOL_ARGS:-}
command -v "$tool" >/dev/null || { printf 'coverage tool not found: %s\n' "$tool" >&2; exit 127; }

directory=$(dirname "$output")
mkdir -p "$directory"
rm -f "$output"
temporary=$(mktemp "$directory/.maintained.XXXXXX")
trap 'rm -f "$temporary"' EXIT

for source_file in "$@"; do
    report=$("$tool" $args -n "$source_file")
    record=$(printf '%s\n' "$report" | awk -v source="File '$source_file'" '
        $0 == source { show = 1 }
        /^File / && $0 != source { show = 0 }
        show && (/^File / || /^Lines executed:/) { print; if (/^Lines executed:/) exit }
    ')
    printf '%s\n' "$record" | grep -Fx "File '$source_file'" >/dev/null
    printf '%s\n' "$record" | grep -Eq '^Lines executed:([0-9]+\.)?[0-9]+% of [1-9][0-9]*$'
    printf '%s\n' "$record" >> "$temporary"
done

grep -Eq "^File '(src|lib/muntarfs)/" "$temporary"
grep -Eq '^Lines executed:([0-9]+\.)?[0-9]+% of [1-9][0-9]*$' "$temporary"
mv "$temporary" "$output"
trap - EXIT
