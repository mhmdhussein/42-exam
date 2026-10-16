#!/bin/bash
# Runs from the repo root. Compares the student's tsp with the reference
# on each case.
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

gcc -Wall -Wextra -Werror -o ref tsp.c -lm || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/tsp/*.c -lm 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(printf "$1" | ./ref | cat -e)
    actual=$(printf "$1" | timeout 20 ./stud | cat -e; [ ${PIPESTATUS[1]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "printf '$1' | ./tsp" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

check '1, 1\n0, 1\n1, 0\n0, 0\n'
check '0, 0\n1, 0\n2, 0\n0, 1\n1, 1\n2, 1\n1, 2\n2, 2\n'
check '0, 0\n3, 4\n'
check '0, 0\n10, 0\n5, 8.66\n'
check '-1.5, 2.25\n3.75, -4\n0.5, 0.5\n7, 7\n-3, -3\n'
check '0, 0\n4, 1\n1, 5\n6, 6\n2, 2\n8, 3\n3, 9\n9, 0\n5, 4\n7, 8\n'
check '2.5, 1\n0, 7.25\n6, 3\n9.5, 9\n1, 1\n4, 8\n8, 0.5\n3, 4.5\n7, 6\n0.5, 3\n5, 5.5\n'

rm -f ref stud .dev
touch passed
