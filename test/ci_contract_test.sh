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
tutorial-examples|make check-tutorial-examples|make tutorial-examples-disabled
CASES

cp .github/workflows/main.yml "$fixture"
sed -i 's/, debian-test,/,/' "$fixture"
if ./test/ci_contract.sh "$fixture"; then printf 'CI contract accepted an ungated shared-libtcc lane\n' >&2; exit 1; fi

cp .github/workflows/main.yml "$fixture"
sed -i '/^permissions:$/,+1d' "$fixture"
if ./test/ci_contract.sh "$fixture"; then printf 'CI contract accepted missing default permissions\n' >&2; exit 1; fi

cp .github/workflows/main.yml "$fixture"
sed -i '0,/      contents: write/s//      actions: write/' "$fixture"
if ./test/ci_contract.sh "$fixture"; then printf 'CI contract accepted overbroad or missing release permissions\n' >&2; exit 1; fi

cp .github/workflows/main.yml "$fixture"
sed -i '/  reuse:/a\    permissions:\n      issues: write' "$fixture"
if ./test/ci_contract.sh "$fixture"; then printf 'CI contract accepted write permission on a test job\n' >&2; exit 1; fi

cp .github/workflows/main.yml "$fixture"
sed -i '/      - uses: actions\/checkout@.*# v7.0.1/{N;/      - name: download binary artifacts/{d;}}' "$fixture"
if ./test/ci_contract.sh "$fixture"; then printf 'CI contract accepted VirusTotal without checkout\n' >&2; exit 1; fi
