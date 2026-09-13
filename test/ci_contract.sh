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

require_job_order() {
    local job=$1 first=$2 second=$3 body first_line second_line
    body=$(job_body "$job") || { printf 'Missing CI job: %s\n' "$job" >&2; exit 1; }
    first_line=$(grep -nF -- "$first" <<<"$body" | head -n1 | cut -d: -f1)
    second_line=$(grep -nF -- "$second" <<<"$body" | head -n1 | cut -d: -f1)
    [[ -n "$first_line" && -n "$second_line" && "$first_line" -lt "$second_line" ]] || {
        printf 'CI job %s must place %s before %s\n' "$job" "$first" "$second" >&2
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
require_job win-msvc-test 'Run MSVC unit tests'
require_job semantic-release 'needs: [linux-test, linux-sanitizer, linux-coverage, linux-dmon, debian-test, osx-native-test, win-mingw-test, win-msvc-test]'
require_job_order virustotal 'actions/checkout@' './test/release_artifact_set.sh'

grep -A2 '^permissions:$' "$workflow" | grep -Fxq '  contents: read' || {
    printf 'Workflow must default to read-only contents permission\n' >&2; exit 1;
}
for job in semantic-release binary-release virustotal remove-tag-on-fail; do
    body=$(job_body "$job")
    grep -Fxq '      contents: write' <<<"$body" || {
        printf 'CI job %s lacks required contents write permission\n' "$job" >&2; exit 1;
    }
done
[[ $(grep -cFx '      contents: write' "$workflow") -eq 4 &&
   $(grep -cFx '      checks: write' "$workflow") -eq 1 &&
   $(grep -cFx '      issues: write' "$workflow") -eq 1 &&
   $(grep -cFx '      pull-requests: write' "$workflow") -eq 1 ]] || {
    printf 'Workflow write permissions exceed the approved release jobs\n' >&2; exit 1;
}
semantic_body=$(job_body semantic-release)
if ! grep -Fxq '      issues: write' <<<"$semantic_body" ||
   ! grep -Fxq '      pull-requests: write' <<<"$semantic_body"; then
    printf 'Semantic release lacks required issue or pull-request permission\n' >&2; exit 1;
fi
c_lint_body=$(job_body c-lint)
grep -Fxq '      checks: write' <<<"$c_lint_body" || {
    printf 'C lint lacks required checks permission\n' >&2; exit 1;
}
if grep -Eq '^[[:space:]]+(actions|deployments|id-token|packages|pages|security-events|statuses): write' "$workflow"; then
    printf 'Workflow grants an unapproved write permission\n' >&2; exit 1
fi

printf 'CI_CONTRACT lanes=8 capabilities=15 workflow=%s\n' "$workflow"
