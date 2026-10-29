#!/bin/bash
# Custom tester (the shared runner can't drop a line for only some cases).
# Runs pipelines through the reference and the student's picoshell (main.c
# splits argv on "|") and compares the output. Runs from the repo root.
source .system/trace.sh
RENDU=${RENDU:-$PWD/rendu/picoshell} # the pushed snapshot in git submission mode
TOTAL=$(grep -c '^\(FILTER=return \)\?check ' "$0")
cd .system/grading || exit 1
rm -f passed

cleanup() { rm -f ref stud main.o ref.o stud.o .dev; }
fail_build() { trace_build "$1" "$2"; cleanup; exit 1; }

# the test main is ours: its warnings don't count
gcc -w -c main.c -o main.o || fail_build "reference did not compile"
gcc -Wall -Wextra -Werror -c picoshell.c -o ref.o && gcc -o ref ref.o main.o \
    || fail_build "reference did not compile"
gcc -Wall -Wextra -Werror -c "$RENDU/picoshell.c" -o stud.o 2> .dev \
    || fail_build "compile error" "$(cat .dev)"
gcc -o stud stud.o main.o 2> .dev || fail_build "compile error" "$(cat .dev)"
for sym in $(nm -u stud.o | awk '{print $NF}'); do
    case "$sym" in __stack_chk_fail | _GLOBAL_OFFSET_TABLE_) continue ;; esac
    grep -qx "$sym" allowed || fail_build "forbidden function: $sym"
done

i=0
# Commands that fail: whether picoshell then returns 0 or 1 is not specified,
# so those cases drop the "return:" line (FILTER=return).
drop_return() {
    if [ "$FILTER" = return ]; then grep -v "^return:"; else cat; fi
}
check() {
    local expected actual status reason command
    i=$((i + 1))
    expected=$(./ref "$@" < /dev/null 2> /dev/null | drop_return | cat -e)
    actual=$(timeout -k 2 10 ./stud "$@" < /dev/null 2> /dev/null > .out; status=$?; drop_return < .out | cat -e; exit $status)
    status=$?
    rm -f .out
    command="./picoshell$(printf ' "%s"' "$@")"
    [ "$FILTER" = return ] && command="$command   (\"return:\" line not compared)"
    reason=$(trace_status_reason "$status" 10)
    [ -z "$reason" ] && [ "$expected" != "$actual" ] && reason=diff
    if [ -n "$reason" ]; then
        trace_case "$i" "$TOTAL" "$reason" "$command" "" "$expected" "$actual"
        cleanup
        exit 1
    fi
}

check echo hello
check /bin/ls main.c "|" grep main
check echo squalala "|" cat "|" sed s/a/b/g
check echo test "|" cat
check echo pipeline "|" cat "|" cat "|" cat "|" cat "|" cat
check echo "hello world" "|" grep world
FILTER=return check echo hello "|" grep xyz
check echo "aaa bbb ccc" "|" sed s/a/x/g "|" sed s/b/y/g "|" sed s/c/z/g
check seq 1 20000 "|" grep 7 "|" wc -l
check yes "|" head -n 3
FILTER=return check ls /nonexistent_dir_xyz "|" cat
FILTER=return check nonexistent_command_xyz
FILTER=return check echo hi "|" nonexistent_command_xyz "|" cat

cleanup
touch passed
