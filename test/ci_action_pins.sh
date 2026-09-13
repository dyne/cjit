#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

for workflow in .github/workflows/*.yml; do
    while IFS= read -r line; do
        [[ "$line" == *'uses:'* ]] || continue
        if [[ ! "$line" =~ @[0-9a-f]{40}[[:space:]]*\#[[:space:]]v[0-9] ]]; then
            printf 'Workflow action is not SHA-pinned with a version comment: %s\n' "$line" >&2
            exit 1
        fi
    done < "$workflow"
done
printf 'CI_ACTION_PINS workflows=%s\n' "$(find .github/workflows -name '*.yml' -type f | wc -l)"
