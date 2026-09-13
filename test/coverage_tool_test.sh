#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT

printf '#!/bin/sh\nprintf "Ubuntu clang version 21.2.3\\n"\n' > "$temporary/clang"
printf '#!/bin/sh\nexit 0\n' > "$temporary/llvm-cov-21"
printf '#!/bin/sh\nexit 0\n' > "$temporary/llvm-cov"
printf '#!/bin/sh\nprintf "gcc (GCC) 15.1.0\\n"\n' > "$temporary/gcc"
printf '#!/bin/sh\nexit 0\n' > "$temporary/gcov"
chmod +x "$temporary/clang" "$temporary/llvm-cov-21" "$temporary/llvm-cov" \
    "$temporary/gcc" "$temporary/gcov"

tool=$(PATH="$temporary:$PATH" ./test/coverage_tool.sh clang)
[[ "$tool" == "$temporary/llvm-cov-21" ]]
rm "$temporary/llvm-cov-21"
tool=$(PATH="$temporary:$PATH" ./test/coverage_tool.sh clang)
[[ "$tool" == "$temporary/llvm-cov" ]]
tool=$(PATH="$temporary:$PATH" ./test/coverage_tool.sh gcc)
[[ "$tool" == "$temporary/gcov" ]]
