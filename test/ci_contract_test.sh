#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
fixture="$temporary/main.yml"
cp .github/workflows/main.yml "$fixture"
./test/ci_contract.sh "$fixture"

# Recreate the fixture for representative Linux, macOS, and Windows/MSVC
# capabilities so runner-family ownership cannot silently disappear.
while IFS='|' read -r label expected replacement; do
    cp .github/workflows/main.yml "$fixture"
    sed -i "s/$expected/$replacement/" "$fixture"
    if ./test/ci_contract.sh "$fixture"; then
        printf 'CI contract accepted removed %s capability\n' "$label" >&2
        exit 1
    fi
done <<'CASES'
linux-fuzz|make fuzz-smoke|make fuzz-disabled
macos-native|make apple-osx|make apple-disabled
windows-msvc|check-unit-msvc|check-unit-disabled
CASES
