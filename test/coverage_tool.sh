#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

compiler=${1:-cc}
version=$($compiler --version 2>/dev/null | sed -n '1p') || {
    printf 'coverage compiler is unavailable: %s\n' "$compiler" >&2
    exit 1
}

case "$version" in
    *clang*)
        major=$(sed -nE 's/.*clang version ([0-9]+).*/\1/p' <<<"$version")
        if [[ -n "$major" ]] && command -v "llvm-cov-$major" >/dev/null; then
            command -v "llvm-cov-$major"
        elif command -v llvm-cov >/dev/null; then
            command -v llvm-cov
        else
            printf 'coverage tool not found for %s\n' "$version" >&2
            exit 127
        fi
        ;;
    *)
        command -v gcov >/dev/null || {
            printf 'coverage tool not found for %s\n' "$version" >&2
            exit 127
        }
        command -v gcov
        ;;
esac
