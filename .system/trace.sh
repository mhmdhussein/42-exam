#!/bin/bash
# Trace helpers shared by .system/run_tests.sh and custom tester.sh files.
# They write the trace format from docs/TESTER_GUIDE.md to <repo root>/traceback.
#   source .system/trace.sh   (or "$(dirname "$0")/../trace.sh")

TRACE_FILE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/traceback"

# A failed case. $1 = case number, $2 = number of cases, $3 = reason,
# $4 = command, $5 = stdin ("" for none); $6/$7 = expected/got, already through cat -e.
trace_case() {
    {
        printf '=== CASE %s/%s FAILED: %s ===\n' "$1" "$2" "$3"
        printf 'COMMAND: %s\n' "$4"
        printf 'STDIN:   %s\n' "${5:-(none)}"
        if [ $# -ge 7 ]; then
            printf 'EXPECTED (cat -e):\n%s\n' "$6"
            printf 'GOT (cat -e):\n%s\n' "$7"
        fi
        printf '=== END CASE %s ===\n' "$1"
    } > "$TRACE_FILE"
}

# A failure before any case runs. $1 = reason (missing file, compile error, forbidden function: x...),
# $2 = details (compiler output), optional.
trace_build() {
    {
        printf '=== FAILED: %s ===\n' "$1"
        [ -n "$2" ] && printf 'GOT:\n%s\n' "$2"
        printf '=== END ===\n'
    } > "$TRACE_FILE"
}

# Reason for a non-zero exit status from `timeout`, or nothing if it is a normal exit.
# $1 = status, $2 = timeout in seconds.
trace_status_reason() {
    if [ "$1" -eq 124 ] || [ "$1" -eq 137 ]; then
        printf 'timeout after %ss' "$2"
    elif [ "$1" -gt 128 ]; then
        printf 'crash: SIG%s' "$(kill -l $(($1 - 128)))"
    fi
}
