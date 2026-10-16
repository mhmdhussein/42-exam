#!/bin/bash
# Runs from the repo root. Runs the reference and the student's sandbox on
# nice and bad functions (see main.c) and compares the output.
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

gcc -Wall -Wextra -Werror -o ref main.c sandbox.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud main.c ../../rendu/sandbox/sandbox.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

names=(nice_fn nice_slow exit_code exit_one segfault do_abort loop ignore_alarm kill_self fpe bus spawn)

# $1 = function index in main.c, $2 = timeout, $3 = verbose
check() {
    local expected actual
    expected=$(./ref "$@" 2>/dev/null | cat -e)
    actual=$(timeout 10 ./stud "$@" 2>/dev/null | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "sandbox(${names[$1]}, $2, $([ "$3" = 1 ] && echo true || echo false))  (see main.c)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

for i in 0 1 2 3 4 5 8 9 10 11; do
    check $i 3 1
done
check 6 1 1
check 7 1 1
check 1 2 0
check 4 2 0
check 6 1 0

rm -f ref stud .dev
touch passed
