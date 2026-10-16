#!/bin/bash
# Runs from the repo root. Compares the student's permutations with the reference
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

gcc -Wall -Wextra -Werror -o ref permutations.c  || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/permutations/*.c  2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(./ref "$@" | cat -e)
    actual=$(timeout 10 ./stud "$@" | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./permutations \"$*\"" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

check a
check ab
check ba
check abc
check cab
check dbca
check xyzab
check hgfedcba

rm -f ref stud .dev
touch passed
