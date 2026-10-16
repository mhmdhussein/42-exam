#!/bin/bash
# Runs every exercise's tester (its own tester.sh, or the shared runner
# .system/run_tests.sh on its cases file) against its reference solution, in an
# isolated copy of the repo. Each exercise must pass with the reference and
# fail with an empty submission and with every known-wrong solution in wrong/.
#   bash .system/check_subjects.sh [filter]   (filter: substring of the exercise path)

ROOT=$(cd "$(dirname "$0")/.." && pwd)
JOBS=${JOBS:-$(nproc)}
TIMEOUT=${TIMEOUT:-180}

# Run one tester in a fresh sandbox; $2 = "ref", "empty" or "wrong:<entry of wrong/>".
# Prints passed/failed/timeout. A wrong/ file replaces the reference's source file;
# a wrong/ folder is the whole submission.
run_tester() {
    local ex=$1 mode=$2 name sandbox result
    name=$(basename "$ex")
    sandbox=$(mktemp -d)
    mkdir -p "$sandbox/.system/grading" "$sandbox/rendu/$name"
    cp "$ROOT"/.system/*.sh "$sandbox/.system/"
    find "$ex" -maxdepth 1 -type f -exec cp {} "$sandbox/.system/grading/" \;
    [ -d "$ex/inputs" ] && cp -r "$ex/inputs" "$sandbox/.system/grading/"
    if [ "$mode" = ref ] || [[ $mode == wrong:* ]]; then
        find "$ex" -maxdepth 1 -type f ! -name tester.sh ! -name beta ! -name cases ! -name allowed \
            ! -name config -exec cp {} "$sandbox/rendu/$name/" \;
    fi
    if [[ $mode == wrong:* ]]; then
        local wrong=$ex/wrong/${mode#wrong:} target
        if [ -d "$wrong" ]; then
            rm -rf "$sandbox/rendu/$name"
            cp -r "$wrong" "$sandbox/rendu/$name"
        else
            target=$(cd "$ex" && ls ./*.c ./*.cpp 2> /dev/null | grep -v '/main\.c' | head -1)
            cp "$wrong" "$sandbox/rendu/$name/${target#./}"
        fi
    fi
    if [ -f "$ex/tester.sh" ]; then
        (cd "$sandbox" && timeout "$TIMEOUT" bash .system/grading/tester.sh) >/dev/null 2>&1
    else
        (cd "$sandbox" && timeout "$TIMEOUT" bash .system/run_tests.sh "$name") >/dev/null 2>&1
    fi
    if [ $? -eq 124 ]; then
        result=timeout
    elif [ -e "$sandbox/.system/grading/passed" ]; then
        result=passed
    else
        result=failed
    fi
    rm -rf "$sandbox"
    echo "$result"
}

check_one() {
    local ex=$1 rel ref empty wrong caught=0 leaky=""
    rel=${ex#"$ROOT"/.subjects/}
    if [ ! -f "$ex/tester.sh" ] && [ ! -f "$ex/cases" ]; then
        echo "NO_TESTER  $rel"
        return
    fi
    ref=$(run_tester "$ex" ref)
    empty=$(run_tester "$ex" empty)
    if [ "$ref" != passed ]; then
        echo "BROKEN     $rel (reference $ref)"
    elif [ "$empty" = passed ]; then
        echo "WEAK       $rel (empty submission passes)"
    else
        for wrong in "$ex"/wrong/*; do
            [ -e "$wrong" ] || continue
            if [ "$(run_tester "$ex" "wrong:${wrong##*/}")" = passed ]; then
                leaky="$leaky ${wrong##*/}"
            else
                caught=$((caught + 1))
            fi
        done
        if [ -n "$leaky" ]; then
            echo "LEAKY      $rel (passes:$leaky)"
        elif [ -d "$ex/wrong" ]; then
            echo "OK         $rel ($caught wrong caught)"
        else
            echo "OK         $rel"
        fi
    fi
}

export -f run_tester check_one
export ROOT TIMEOUT

find "$ROOT/.subjects" -mindepth 4 -maxdepth 4 -type d | grep -- "${1:-}" | sort \
    | xargs -P "$JOBS" -I{} bash -c 'check_one "$@"' _ {} | sort > "${TMPDIR:-/tmp}/check_subjects.$$"

cat "${TMPDIR:-/tmp}/check_subjects.$$"
echo
for s in OK BROKEN WEAK LEAKY NO_TESTER; do
    printf "%-10s %s\n" "$s" "$(grep -c "^$s " "${TMPDIR:-/tmp}/check_subjects.$$")"
done
! grep -qE "^(BROKEN|WEAK|LEAKY) " "${TMPDIR:-/tmp}/check_subjects.$$"
status=$?
rm -f "${TMPDIR:-/tmp}/check_subjects.$$"
exit $status
