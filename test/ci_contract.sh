#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Verify that every CI lane owns an executable test capability.  Keep this
# manifest intentionally small: it protects invocation contracts, while the
# invoked Make targets remain responsible for individual test counts.
set -Eeuo pipefail

workflow=${1:-.github/workflows/main.yml}
test -r "$workflow" || { printf 'CI workflow is unreadable: %s\n' "$workflow" >&2; exit 1; }

job_body() {
    local job=$1
    awk -v job="$job" '
        $0 == "  " job ":" { found = 1; next }
        found && /^  [A-Za-z0-9_-]+:/ { exit }
        found { print }
        END { if (!found) exit 2 }
    ' "$workflow"
}

require_job() {
    local job=$1 pattern=$2 body
    body=$(job_body "$job") || { printf 'Missing CI job: %s\n' "$job" >&2; exit 1; }
    grep -Fq -- "$pattern" <<<"$body" || {
        printf 'CI job %s is missing required capability: %s\n' "$job" "$pattern" >&2
        exit 1
    }
}

# Linux owns both native and self-host compiler runs; the sanitizer lane owns
# the bounded fuzz replay.  Platform lanes use CJIT_REQUIRED_PLATFORM so a
# legitimate host-specific skip cannot silently become a zero-case success.
require_job linux-test 'make linux CC=clang'
require_job linux-test 'CJIT_REQUIRED_PLATFORM=linux make check-ci'
require_job linux-test 'make linux CC=cjit'
require_job linux-sanitizer 'make debug-asan CC=clang'
require_job linux-sanitizer 'make fuzz-smoke'
require_job linux-coverage 'make coverage CC=clang'
require_job linux-dmon 'CJIT_REQUIRED_PLATFORM=linux make run-dmon-suite'
require_job debian-test 'make meson'
require_job debian-test 'make check-ci'
require_job osx-native-test 'make apple-osx'
require_job osx-native-test 'CJIT_REQUIRED_PLATFORM: macos'
require_job win-mingw-test 'make win-mingw'
require_job win-mingw-test 'CJIT_REQUIRED_PLATFORM: windows'
require_job win-msvc-test 'make win-msvc'
require_job win-msvc-test 'check-unit-msvc'

printf 'CI_CONTRACT lanes=8 capabilities=15 workflow=%s\n' "$workflow"
