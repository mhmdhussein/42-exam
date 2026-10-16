#!/bin/bash
# Shared tester: grades rendu/<name>/ against the reference solution, using the
# exercise files copied to .system/grading/ (cases, allowed, config, inputs/).
# Rules: docs/TESTER_GUIDE.md. Runs from the repo root.
#   bash .system/run_tests.sh <name>
# On failure: writes ./traceback and exits 1. On success: touches .system/grading/passed.

NAME=$1
ROOT=$(pwd)
GRADING=$ROOT/.system/grading
RENDU=$ROOT/rendu/$NAME
source "$ROOT/.system/trace.sh"

cd "$GRADING" || exit 1
rm -f passed "$TRACE_FILE"

# ---- config ----------------------------------------------------------------
TIMEOUT=10 CFLAGS_EXTRA="" BUFFER_SIZES="" CHECK_EXIT=off CHECK_STDERR=off CHECK_LEAKS=off
if [ -f config ]; then
    while IFS='=' read -r key value; do
        case "$key" in
            timeout) TIMEOUT=$value ;;
            cflags) CFLAGS_EXTRA=$value ;;
            buffer_sizes) BUFFER_SIZES=$value ;;
            exit_code) CHECK_EXIT=$value ;;
            stderr) CHECK_STDERR=$value ;;
            leaks) CHECK_LEAKS=$value ;;
        esac
    done < <(grep -v '^[[:space:]]*\(#\|$\)' config)
fi
if [ "$CHECK_LEAKS" = on ] && ! command -v valgrind > /dev/null; then
    echo "warning: valgrind is not installed, skipping the leak check"
    CHECK_LEAKS=off
fi

# ---- sources ---------------------------------------------------------------
# Reference sources: every top-level .c/.cpp except the test main. The student
# must hand in files with the same names.
SOURCES=() MAIN=""
for f in *.c *.cpp; do
    [ -f "$f" ] || continue
    case "$f" in
        main.c | main.cpp) MAIN=$f ;;
        *) SOURCES+=("$f") ;;
    esac
done
if ls ./*.cpp > /dev/null 2>&1; then
    CC=(c++ -std=c++98)
else
    CC=(gcc)
fi
read -ra CC_EXTRA <<< "$CFLAGS_EXTRA"
WARN=(-Wall -Wextra -Werror)

cleanup() {
    rm -rf "$GRADING/build_ref" "$GRADING/build_stud" "$GRADING/run"
}

fail_build() {
    trace_build "$1" "$2"
    cleanup
    exit 1
}

# $1 = build dir, $2 = extra flag (BUFFER_SIZE) or "". Builds build dir/prog.
# The solution's files get the strict flags; the test main is ours, so its
# warnings are not the student's problem (-w).
build() {
    local dir=$1 flag=$2
    (
        cd "$dir" || exit 1
        objects=()
        for f in "${SOURCES[@]}"; do
            "${CC[@]}" "${WARN[@]}" "${CC_EXTRA[@]}" $flag -I . -c "$f" -o "$f.o" || exit 1
            objects+=("$f.o")
        done
        if [ -n "$MAIN" ]; then
            "${CC[@]}" -w "${CC_EXTRA[@]}" $flag -I . -c "$MAIN" -o main.o || exit 1
            objects+=(main.o)
        fi
        "${CC[@]}" -o prog "${objects[@]}"
    ) 2>&1
}

# ---- prepare build folders -------------------------------------------------
# Reference: everything in .system/grading. Student: the given headers and
# test main, then the student's files on top (their headers win).
cleanup
mkdir -p build_ref build_stud run
find . -maxdepth 1 -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
    -exec cp {} build_ref/ \;
find . -maxdepth 1 -type f \( -name '*.h' -o -name '*.hpp' \) -exec cp {} build_stud/ \;
[ -n "$MAIN" ] && cp "$MAIN" build_stud/
for f in "${SOURCES[@]}"; do
    [ -f "$RENDU/$f" ] || fail_build "compile error" "missing file: rendu/$NAME/$f"
done
find "$RENDU" -maxdepth 1 -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
    ! -name "$MAIN" -exec cp {} build_stud/ \; 2> /dev/null

# ---- allowed functions -----------------------------------------------------
# Undefined symbols of the student's object files must be in `allowed` or be
# added by the compiler. -fno-builtin stops gcc from turning printf into puts.
undefined_symbols() {
    local dir=$1 f NM_FLAG=""
    [ -n "$BUFFER_SIZES" ] && NM_FLAG="-D BUFFER_SIZE=${BUFFER_SIZES%% *}"
    (
        cd "$dir" || exit
        mkdir -p nm
        for f in "${SOURCES[@]}"; do
            "${CC[@]}" "${WARN[@]}" "${CC_EXTRA[@]}" $NM_FLAG -fno-builtin -I . -c "$f" -o "nm/$f.o" 2> /dev/null
        done
        # calls between the solution's own files are not external functions
        comm -23 <(nm -u nm/*.o 2> /dev/null | awk 'NF {print $NF}' | sort -u) \
            <(nm --defined-only nm/*.o 2> /dev/null | awk 'NF == 3 {print $3}' | sort -u)
        rm -rf nm
    )
}

check_allowed() {
    local ref_symbols sym
    [ -f allowed ] || return
    ref_symbols=$(undefined_symbols build_ref)
    while read -r sym; do
        [ -z "$sym" ] && continue
        case "$sym" in
            __stack_chk_fail | __stack_chk_guard | _GLOBAL_OFFSET_TABLE_) continue ;;
            __cxa_* | __gxx_personality_v0 | _Unwind_Resume | _Zn* | _Zd* | _ZNSt* | _ZSt* | _ZNKSt*) continue ;;
            memcpy | memset | memmove) grep -qx "$sym" <<< "$ref_symbols" && continue ;;
        esac
        grep -qx "$sym" allowed || fail_build "forbidden function: $sym"
    done <<< "$(undefined_symbols build_stud)"
}

# ---- cases -----------------------------------------------------------------
mapfile -t CASES < <(grep -v '^[[:space:]]*\(#\|$\)' cases)
TOTAL=${#CASES[@]}

# $1 = prog, $2 = stdin file, rest = args. Sets STATUS; output in run/out, run/err.
run_prog() {
    local prog=$1 input=$2
    shift 2
    timeout -k 2 "$TIMEOUT" "$prog" "$@" < "$input" > run/out 2> run/err
    STATUS=$?
}

# $1 = stdin file, rest = args. False if valgrind finds definitely lost memory.
leak_free() {
    local input=$1
    shift
    timeout -k 2 "$((TIMEOUT * 5))" valgrind -q --leak-check=full --errors-for-leak-kinds=definite \
        build_stud/prog "$@" < "$input" > /dev/null 2> run/valgrind
    ! grep -q 'definitely lost: [1-9]' run/valgrind
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

        run_prog build_ref/prog "$input" "$@"
        exp_status=$STATUS
        exp_out=$(cat -e run/out)
        exp_err=$(cat -e run/err)
        run_prog build_stud/prog "$input" "$@"
        got_out=$(cat -e run/out)
        got_err=$(cat -e run/err)

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
    out=$(build build_ref "$flag") || fail_build "reference did not compile" "$out"
    out=$(build build_stud "$flag") || fail_build "compile error${bs:+ (BUFFER_SIZE=$bs)}" "$out"
    [ -n "$checked" ] || { check_allowed; checked=1; }
    run_cases "$bs" || { cleanup; exit 1; }
done

cleanup
touch passed
