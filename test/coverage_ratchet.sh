#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

report=${1:?usage: coverage_ratchet.sh REPORT BASELINE}
baseline=${2:?usage: coverage_ratchet.sh REPORT BASELINE}
test -s "$report" && test -r "$baseline"

baseline_percent=$(awk -F= '$1 == "maintained_percent" { print $2 }' "$baseline")
tolerance_percent=$(awk -F= '$1 == "tolerance_percent" { print $2 }' "$baseline")
[[ "$baseline_percent" =~ ^[0-9]+([.][0-9]+)?$ && "$tolerance_percent" =~ ^[0-9]+([.][0-9]+)?$ ]] || {
    printf 'Invalid maintained coverage baseline: %s\n' "$baseline" >&2; exit 1;
}

result=$(awk -v baseline="$baseline_percent" -v tolerance="$tolerance_percent" '
    NR == FNR { if ($0 ~ /^(src\/|lib\/muntarfs\/)/) { split($0, pair, "="); by_file[pair[1]] = pair[2] } next }
    /^File '\''(src|lib\/muntarfs)\// { last_source = $0; sub(/^File '\''/, "", last_source); sub(/'\''$/, "", last_source); keep = 1; next }
    /^File / { keep = 0; next }
    keep && /^Lines executed:/ {
        sub(/^Lines executed:/, ""); split($0, percent, "% of "); split(percent[2], lines, " ");
        source = last_source; total += lines[1]; covered += (percent[1] * lines[1] / 100);
        if (source in by_file && (percent[1] - by_file[source] > .005 || by_file[source] - percent[1] > .005)) {
            delta[source] = sprintf("DELTA file=%s current=%.2f baseline=%.2f delta=%+.2f", source, percent[1], by_file[source], percent[1] - by_file[source]);
            order[++delta_count] = source;
        }
        files++; keep = 0;
    }
    END {
        if (!files || !total) exit 2;
        current = covered * 100 / total; minimum = baseline - tolerance;
        printf "current=%.2f minimum=%.2f files=%d", current, minimum, files;
        for (i = 1; i <= delta_count; i++) for (j = i + 1; j <= delta_count; j++) if (order[j] < order[i]) { swap = order[i]; order[i] = order[j]; order[j] = swap }
        for (i = 1; i <= delta_count; i++) print "\n" delta[order[i]];
        if (current + 0.000001 < minimum) exit 1;
    }
' "$baseline" "$report") || { printf 'Maintained coverage ratchet failed: %s\n' "$result" >&2; exit 1; }
printf 'COVERAGE_RATCHET %s\n' "$result"
