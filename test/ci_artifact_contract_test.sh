#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
fixture="$temporary/main.yml"
cp .github/workflows/main.yml "$fixture"
./test/ci_artifact_contract.sh "$fixture"
sed -i '0,/if-no-files-found: error/s//if-no-files-found: warn/' "$fixture"
if ./test/ci_artifact_contract.sh "$fixture"; then
    printf 'Artifact contract accepted a permissive upload\n' >&2
    exit 1
fi
