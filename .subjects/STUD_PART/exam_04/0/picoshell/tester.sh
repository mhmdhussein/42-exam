#!/bin/bash
# Runs from the repo root. Runs pipelines through the reference and the
# student's picoshell (main.c splits argv on "|") and compares the output.
cd .system/grading || exit 1
rm -f traceback passed

fail() {
    {
        echo "----------------8<-------------[ START TEST "
        printf '%s\n' "$@"
        echo "----------------8<------------- END TEST ]"
    } > traceback
    rm -f ref stud .dev
    mv traceback ../../traceback
    exit 1
}

gcc -Wall -Wextra -Werror -o ref main.c picoshell.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud main.c ../../rendu/picoshell/picoshell.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# Commands that fail: whether picoshell then returns 0 or 1 is not specified,
# so those cases drop the "return:" line (FILTER=return).
drop_return() {
    if [ "$FILTER" = return ]; then grep -v "^return:"; else cat; fi
}

check() {
    local expected actual
    expected=$(./ref "$@" < /dev/null 2>/dev/null | drop_return | cat -e)
    actual=$(timeout 10 ./stud "$@" < /dev/null 2>/dev/null | drop_return | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./picoshell $(printf "'%s' " "$@")" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
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

rm -f ref stud .dev
touch passed
