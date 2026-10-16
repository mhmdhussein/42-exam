#!/bin/bash
# Runs from the repo root. Compares the student's powerset with the reference
# on each case (line order does not matter).
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

gcc -Wall -Wextra -Werror -o ref powerset.c  || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/powerset/*.c  2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(./ref "$@" | sort | cat -e)
    actual=$(timeout 10 ./stud "$@" | sort | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./powerset $*" \
        "        🔎 YOUR OUTPUT (sorted):" "$actual" "        🗝 EXPECTED OUTPUT (sorted):" "$expected"
}

check 5 1 2 3 4 5
check 3 1 0 2 4 5 3
check 12 5 2 1 8 4 3 7 11
check 0 1 -1
check 7 3 8 2
check 0
check 100 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
check -1 1 2 3 4 5 -10
check 0 -1 1 2 3 -2
check 13 65 23 3 4 6 7 1 2
check 10 0 1 2 3 4 5 6 7 8 9

rm -f ref stud .dev
touch passed
