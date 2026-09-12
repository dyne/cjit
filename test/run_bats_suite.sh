#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Dyne.org Foundation
# SPDX-License-Identifier: GPL-3.0-or-later

# Run one Bats file and publish an auditable executed/skipped summary.  The
# optional second argument makes a zero-execution suite a hard failure.
set -euo pipefail

suite=${1:?usage: run_bats_suite.sh SUITE [require-execution]}
require_execution=${2:-0}
report=$(mktemp)
trap 'rm -f "$report"' EXIT

if ! ./test/bats/bin/bats "$suite" | tee "$report"; then
    exit 1
fi

passed=$(awk '/^ok [0-9]+ / { count += 1 } END { print count + 0 }' "$report")
skipped=$(awk '/^ok [0-9]+ .*# skip/ { count += 1 } END { print count + 0 }' "$report")
executed=$((passed - skipped))

printf 'BATS_SUITE suite=%s executed=%s skipped=%s\n' "$suite" "$executed" "$skipped"
if [[ "$require_execution" == 1 && "$executed" -eq 0 ]]; then
    printf 'Required Bats suite did not execute any cases: %s\n' "$suite" >&2
    exit 1
fi
