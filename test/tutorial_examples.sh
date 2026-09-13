#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

repository_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$repository_root"

for command_name in curl awk grep; do
    command -v "$command_name" >/dev/null || {
        printf 'Tutorial example test requires %s\n' "$command_name" >&2
        exit 1
    }
done
[[ -x ./cjit ]] || { printf 'Build ./cjit before testing tutorial examples\n' >&2; exit 1; }
[[ -r /usr/include/SDL2/SDL.h ]] || {
    printf 'Tutorial example test requires SDL2 development headers\n' >&2
    exit 1
}

source build/demo-dependencies.sh
temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
include_dir="$temporary/include"
binary_dir="$temporary/bin"
mkdir -p "$include_dir" "$binary_dir"

download_header() {
    local output=$1
    local url=$2
    curl --fail --silent --show-error --location --retry 3 \
        --output "$include_dir/$output" "$url"
    [[ -s "$include_dir/$output" ]] || {
        printf 'Downloaded tutorial header is empty: %s\n' "$output" >&2
        exit 1
    }
}

cp test/dmon.h "$include_dir/dmon.h"
download_header nuklear.h \
    "https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/${NUKLEAR_REF}/nuklear.h"
download_header miniaudio.h \
    "https://raw.githubusercontent.com/mackron/miniaudio/${MINIAUDIO_REF}/miniaudio.h"
download_header termbox2.h \
    "https://raw.githubusercontent.com/termbox/termbox2/${TERMBOX2_REF}/termbox2.h"

awk '
    $0 == "cat << EOF > hello.c" { copying = 1; next }
    copying && $0 == "EOF" { exit }
    copying { print }
' docs/tutorial.md > "$temporary/hello.c"
[[ -s "$temporary/hello.c" ]] || {
    printf 'Could not extract hello.c from docs/tutorial.md\n' >&2
    exit 1
}
hello_output=$(./cjit -q "$temporary/hello.c" 2>&1)
grep -Fq 'Hello, World!' <<<"$hello_output" || {
    printf 'Tutorial hello.c produced unexpected output: %s\n' "$hello_output" >&2
    exit 1
}
printf 'TUTORIAL_EXAMPLE name=hello contract=executed\n'

while IFS='|' read -r name documentation; do
    grep -Fq "$name.c" "$documentation" || {
        printf 'Tutorial no longer references examples/%s.c in %s\n' "$name" "$documentation" >&2
        exit 1
    }
    output="$binary_dir/$name"
    build_log="$temporary/$name.log"
    if ! ./cjit -q -I"$include_dir" -I/usr/include -o "$output" \
        "examples/$name.c" >"$build_log" 2>&1; then
        printf 'Tutorial example failed to link: %s\n' "$name" >&2
        sed -n '1,120p' "$build_log" >&2
        exit 1
    fi
    if grep -Fq 'undefined dynamic symbol' "$build_log"; then
        printf 'Tutorial example linked with unresolved symbols: %s\n' "$name" >&2
        sed -n '1,120p' "$build_log" >&2
        exit 1
    fi
    [[ -s "$output" && -x "$output" ]] || {
        printf 'Tutorial example did not produce an executable: %s\n' "$name" >&2
        exit 1
    }
    printf 'TUTORIAL_EXAMPLE name=%s contract=linked\n' "$name"
done <<'EXAMPLES'
donut|docs/tutorial.md
life|docs/tutorial.md
dmon|docs/filesystem.md
sdl2_noise|docs/graphics.md
opengl|docs/graphics.md
nuklear|docs/graphics.md
miniaudio|docs/sound.md
termbox2|docs/tui.md
EXAMPLES

dmon_output=$("$binary_dir/dmon")
grep -Fq 'usage: test dirname' <<<"$dmon_output" || {
    printf 'Tutorial dmon example did not execute its no-argument contract\n' >&2
    exit 1
}
printf 'TUTORIAL_EXAMPLES executed=2 linked=8 total=9\n'
