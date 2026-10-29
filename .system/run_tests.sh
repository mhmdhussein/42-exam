#!/bin/bash
# Shared tester: grades rendu/<name>/ against the reference solution, using the
# exercise files copied to .system/grading/ (cases, allowed, config, inputs/).
# Rules: docs/TESTER_GUIDE.md. Runs from the repo root.
#   bash .system/run_tests.sh <name>
# On failure: writes ./traceback and exits 1. On success: touches .system/grading/passed.
# While it runs, .system/grading/progress holds the step (compiling, test 3/8) for the
# examshell spinner; on success .system/grading/result holds tests=<number of cases>.

NAME=$1
ROOT=$(pwd)
GRADING=$ROOT/.system/grading
RENDU=${RENDU:-$ROOT/rendu/$NAME} # git submission mode points it at the pushed snapshot
source "$ROOT/.system/trace.sh"

cd "$GRADING" || exit 1
rm -f passed result progress "$TRACE_FILE"
progress() { printf '%s\n' "$1" > "$GRADING/progress"; }

# ---- config ----------------------------------------------------------------
TIMEOUT=10 CFLAGS_EXTRA="" LDFLAGS_EXTRA="" BUFFER_SIZES="" FILES=""
CHECK_EXIT=off CHECK_STDERR=off CHECK_LEAKS=off SORT_OUTPUT=off IGNORE=""
if [ -f config ]; then
    while IFS='=' read -r key value; do
        case "$key" in
            timeout) TIMEOUT=$value ;;
            cflags) CFLAGS_EXTRA=$value ;;
            ldflags) LDFLAGS_EXTRA=$value ;;
            buffer_sizes) BUFFER_SIZES=$value ;;
            files) FILES=$value ;;
            exit_code) CHECK_EXIT=$value ;;
            stderr) CHECK_STDERR=$value ;;
            leaks) CHECK_LEAKS=$value ;;
            sort_output) SORT_OUTPUT=$value ;;
            ignore) IGNORE=$value ;;
        esac
    done < <(grep -v '^[[:space:]]*\(#\|$\)' config)
fi
if [ "$CHECK_LEAKS" = on ] && ! command -v valgrind > /dev/null; then
    echo "warning: valgrind is not installed, skipping the leak check"
    CHECK_LEAKS=off
fi

# ---- sources ---------------------------------------------------------------
# REF_SOURCES / STUD_SOURCES get the strict flags. GIVEN_SOURCES are ours (the
# test main, files handed to the student) and are built in both, with -w.
#   no `files`:            the student hands in every top-level .c/.cpp except
#                          main.c/main.cpp (the test main), with the same names.
#   files=a.cpp a.hpp:     the student hands in exactly these; other sources are given.
#   files=*.c *.h:         the student hands in any files matching; no test main.
ALL_SOURCES=()
for f in *.c *.cpp; do
    [ -f "$f" ] && ALL_SOURCES+=("$f")
done
REF_SOURCES=() GIVEN_SOURCES=() STUD_SOURCES=()
read -ra FILE_LIST <<< "$FILES"
is_source() { [[ $1 == *.c || $1 == *.cpp ]]; }
if [ -z "$FILES" ]; then
    for f in "${ALL_SOURCES[@]}"; do
        case "$f" in
            main.c | main.cpp) GIVEN_SOURCES+=("$f") ;;
            *) REF_SOURCES+=("$f") ;;
        esac
    done
    STUD_SOURCES=("${REF_SOURCES[@]}")
    FILE_LIST=("${REF_SOURCES[@]}")
elif [[ $FILES == *'*'* ]]; then
    REF_SOURCES=("${ALL_SOURCES[@]}")
else
    for f in "${ALL_SOURCES[@]}"; do
        if [[ " $FILES " == *" $f "* ]]; then REF_SOURCES+=("$f"); else GIVEN_SOURCES+=("$f"); fi
    done
    STUD_SOURCES=("${REF_SOURCES[@]}")
fi
if [ ${#ALL_SOURCES[@]} -gt 0 ] && ls ./*.cpp > /dev/null 2>&1; then
    CC=(c++ -std=c++98)
else
    CC=(gcc)
fi
read -ra CC_EXTRA <<< "$CFLAGS_EXTRA"
read -ra LD_EXTRA <<< "$LDFLAGS_EXTRA"
WARN=(-Wall -Wextra -Werror)

cleanup() {
    rm -rf "$GRADING/build_ref" "$GRADING/build_stud" "$GRADING/run"
}

fail_build() {
    trace_build "$1" "$2"
    cleanup
    exit 1
}

# $1 = build dir, $2 = extra flag (BUFFER_SIZE) or "", rest = strict sources.
# Builds <dir>/prog. The given sources are ours, so their warnings are not the
# student's problem (-w).
build() {
    local dir=$1 flag=$2
    shift 2
    (
        cd "$dir" || exit 1
        objects=()
        for f in "$@"; do
            "${CC[@]}" "${WARN[@]}" "${CC_EXTRA[@]}" $flag -I . -c "$f" -o "$f.o" || exit 1
            objects+=("$f.o")
        done
        for f in "${GIVEN_SOURCES[@]}"; do
            "${CC[@]}" -w "${CC_EXTRA[@]}" $flag -I . -c "$f" -o "$f.given.o" || exit 1
            objects+=("$f.given.o")
        done
        "${CC[@]}" -o prog "${objects[@]}" "${LD_EXTRA[@]}"
    ) 2>&1
}

# ---- prepare build folders -------------------------------------------------
# Reference: every source and header here. Student: the given sources and the
# headers they didn't have to write, then the student's files on top.
cleanup
mkdir -p build_ref build_stud run
find . -maxdepth 1 -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
    -exec cp {} build_ref/ \;
for f in *.h *.hpp "${GIVEN_SOURCES[@]}"; do
    [ -f "$f" ] || continue
    [[ $FILES == *'*'* ]] && [[ $f == *.h || $f == *.hpp ]] && continue
    [[ " ${FILE_LIST[*]} " == *" $f "* ]] && [ -n "$FILES" ] && continue
    cp "$f" build_stud/
done
if [[ $FILES == *'*'* ]]; then
    for pattern in "${FILE_LIST[@]}"; do
        for f in "$RENDU"/$pattern; do
            [ -f "$f" ] || continue
            cp "$f" build_stud/
            is_source "$f" && STUD_SOURCES+=("${f##*/}")
        done
    done
    [ ${#STUD_SOURCES[@]} -gt 0 ] || fail_build "missing file" "no source files in rendu/$NAME/ (expected: $FILES)"
else
    for f in "${FILE_LIST[@]}"; do
        [ -f "$RENDU/$f" ] || fail_build "missing file" "rendu/$NAME/$f"
        cp "$RENDU/$f" build_stud/
    done
    # the student's own headers win over the given ones
    find "$RENDU" -maxdepth 1 -type f \( -name '*.h' -o -name '*.hpp' \) -exec cp {} build_stud/ \; 2> /dev/null
fi

# ---- allowed functions -----------------------------------------------------
# Undefined symbols of the solution's object files must be in `allowed` or be
# added by the compiler. -fno-builtin stops gcc from turning printf into puts.
# $1 = dir, rest = sources.
undefined_symbols() {
    local dir=$1 f nm_flag=""
    shift
    [ -n "$BUFFER_SIZES" ] && nm_flag="-D BUFFER_SIZE=${BUFFER_SIZES%% *}"
    (
        cd "$dir" || exit
        mkdir -p nm nm_given
        for f in "$@"; do
            "${CC[@]}" "${WARN[@]}" "${CC_EXTRA[@]}" $nm_flag -fno-builtin -I . -c "$f" -o "nm/$f.o" 2> /dev/null
        done
        for f in "${GIVEN_SOURCES[@]}"; do
            "${CC[@]}" -w "${CC_EXTRA[@]}" $nm_flag -I . -c "$f" -o "nm_given/$f.o" 2> /dev/null
        done
        # calls to the solution's own or the given files are not external functions
        comm -23 <(nm -u nm/*.o 2> /dev/null | awk 'NF && !/:$/ {print $NF}' | sort -u) \
            <(nm --defined-only nm/*.o nm_given/*.o 2> /dev/null | awk 'NF == 3 {print $3}' | sort -u)
        rm -rf nm nm_given
    )
}

check_allowed() {
    local ref_symbols sym
    [ -f allowed ] || return
    ref_symbols=$(undefined_symbols build_ref "${REF_SOURCES[@]}")
    while read -r sym; do
        [ -z "$sym" ] && continue
        # glibc renames the scanf family (__isoc99_fscanf, __isoc23_sscanf...)
        [[ $sym =~ ^__isoc[0-9]+_(.*)$ ]] && sym=${BASH_REMATCH[1]}
        case "$sym" in
            __stack_chk_fail | __stack_chk_guard | _GLOBAL_OFFSET_TABLE_) continue ;;
            __cxa_* | __gxx_personality_v0 | _Unwind_Resume | _Zn* | _Zd* | _ZNSt* | _ZSt* | _ZNKSt*) continue ;;
            # is*/to* from <ctype.h> are macros over these tables
            __ctype_b_loc | __ctype_tolower_loc | __ctype_toupper_loc)
                grep -qE '^(is|to)[a-z]+$' allowed && continue ;;
            __errno_location) grep -qx errno allowed && continue ;;
            memcpy | memset | memmove) grep -qx "$sym" <<< "$ref_symbols" && continue ;;
        esac
        grep -qx "$sym" allowed || fail_build "forbidden function: $sym"
    done <<< "$(undefined_symbols build_stud "${STUD_SOURCES[@]}")"
}

# ---- cases -----------------------------------------------------------------
mapfile -t CASES < <(grep -v '^[[:space:]]*\(#\|$\)' cases)
TOTAL=${#CASES[@]}

# $1 = prog, $2 = stdin file (a .sh file is run and its output piped in), rest = args.
# Sets STATUS; output in run/out, run/err.
run_prog() {
    local prog=$1 input=$2
    shift 2
    if [[ $input == *.sh ]]; then
        bash "$input" | timeout -k 2 "$TIMEOUT" "$prog" "$@" > run/out 2> run/err
        STATUS=${PIPESTATUS[1]}
    else
        timeout -k 2 "$TIMEOUT" "$prog" "$@" < "$input" > run/out 2> run/err
        STATUS=$?
    fi
}

# Output as compared: `ignore` lines dropped, sorted if `sort_output`, through cat -e.
shown() {
    if [ -n "$IGNORE" ]; then grep -Ev -- "$IGNORE" "$1"; else cat "$1"; fi \
        | if [ "$SORT_OUTPUT" = on ]; then LC_ALL=C sort; else cat; fi | cat -e
}

# $1 = stdin file, rest = args. False if valgrind finds definitely lost memory.
leak_free() {
    local input=$1
    shift
    [[ $input == *.sh ]] && { bash "$input" > run/stdin; input=run/stdin; }
    timeout -k 2 "$((TIMEOUT * 5))" valgrind -q --leak-check=full --errors-for-leak-kinds=definite \
        build_stud/prog "$@" < "$input" > /dev/null 2> run/valgrind
    ! grep -q 'definitely lost' run/valgrind
}

# Shell-quotes one argument for the trace: "text" when that is exact, else $'...'.
quote_arg() {
    if [[ $1 =~ ^[[:print:]]*$ ]] && [[ $1 != *[\"\$\`\\]* ]]; then
        printf '"%s"' "$1"
    else
        printf '%q' "$1"
    fi
}

# $1 = BUFFER_SIZE or "" (shown in the trace)
run_cases() {
    local bs=$1 i=0 line input arg command stdin_label reason
    local exp_out exp_err exp_status got_out got_err
    for line in "${CASES[@]}"; do
        i=$((i + 1))
        progress "test $i/$TOTAL"
        input=/dev/null
        if [[ $line == stdin:* ]]; then
            eval "set -- ${line#stdin:}"
            input=inputs/$1
            shift
        elif [ "$line" = "(no args)" ]; then
            set --
        else
            eval "set -- $line"
        fi
        command="./$NAME"
        for arg in "$@"; do
            command="$command $(quote_arg "$arg")"
        done
        [ -n "$bs" ] && command="$command   (BUFFER_SIZE=$bs)"
        stdin_label=""
        [ "$input" != /dev/null ] && stdin_label=$input
        [[ $input == *.sh ]] && stdin_label="output of $input"
        [ "$SORT_OUTPUT" = on ] && stdin_label="${stdin_label:-(none)}   (output lines sorted before comparing)"

        run_prog build_ref/prog "$input" "$@"
        exp_status=$STATUS
        exp_out=$(shown run/out)
        exp_err=$(shown run/err)
        run_prog build_stud/prog "$input" "$@"
        got_out=$(shown run/out)
        got_err=$(shown run/err)

        reason=$(trace_status_reason "$STATUS" "$TIMEOUT")
        if [ -n "$reason" ] && [ "$exp_status" -le 128 ]; then
            trace_case "$i" "$TOTAL" "$reason" "$command" "$stdin_label" "$exp_out" "$got_out"
        elif [ "$exp_out" != "$got_out" ]; then
            trace_case "$i" "$TOTAL" diff "$command" "$stdin_label" "$exp_out" "$got_out"
        elif [ "$CHECK_STDERR" = on ] && [ "$exp_err" != "$got_err" ]; then
            trace_case "$i" "$TOTAL" "stderr differs" "$command" "$stdin_label" "$exp_err" "$got_err"
        elif [ "$CHECK_EXIT" = on ] && [ "$exp_status" != "$STATUS" ]; then
            trace_case "$i" "$TOTAL" "exit code differs" "$command" "$stdin_label" \
                "exit code $exp_status" "exit code $STATUS"
        elif [ "$CHECK_LEAKS" = on ] && ! leak_free "$input" "$@"; then
            trace_case "$i" "$TOTAL" leaks "$command" "$stdin_label" "no leaks" "$(cat run/valgrind)"
        else
            continue
        fi
        return 1
    done
}

# One pass per BUFFER_SIZE (a single pass without one if the config has none).
for bs in ${BUFFER_SIZES:-NONE}; do
    flag=""
    [ "$bs" != NONE ] && flag="-D BUFFER_SIZE=$bs" || bs=""
    progress compiling
    out=$(build build_ref "$flag" "${REF_SOURCES[@]}") || fail_build "reference did not compile" "$out"
    out=$(build build_stud "$flag" "${STUD_SOURCES[@]}") || fail_build "compile error${bs:+ (BUFFER_SIZE=$bs)}" "$out"
    [ -n "$checked" ] || { check_allowed; checked=1; }
    run_cases "$bs" || { cleanup; exit 1; }
done

cleanup
rm -f progress
printf 'tests=%s\n' "$TOTAL" > result
touch passed
