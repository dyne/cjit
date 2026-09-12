#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
report="$temporary/maintained.txt"

if COVERAGE_TOOL=not-a-tool ./test/coverage_report.sh "$report" src/main.c; then exit 1; fi
test ! -e "$report"
if COVERAGE_TOOL=/bin/false ./test/coverage_report.sh "$report" src/main.c; then exit 1; fi
test ! -e "$report"
if COVERAGE_TOOL=/bin/true ./test/coverage_report.sh "$report" src/main.c; then exit 1; fi
test ! -e "$report"
if COVERAGE_TOOL=./test/coverage_fake_tool.sh COVERAGE_FAKE_OMIT=src/cjit.c \
    ./test/coverage_report.sh "$report" src/main.c src/cjit.c; then exit 1; fi
test ! -e "$report"
COVERAGE_TOOL=./test/coverage_fake_tool.sh ./test/coverage_report.sh "$report" src/main.c src/cjit.c
test -s "$report"
