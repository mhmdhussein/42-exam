#!/bin/bash
# Runs from the repo root. Compares the student's n_queens with the reference
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

gcc -Wall -Wextra -Werror -o ref n_queens.c  || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/n_queens/*.c  2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(./ref "$@" | sort | cat -e)
    actual=$(timeout 10 ./stud "$@" | sort | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./n_queens $*" \
        "        🔎 YOUR OUTPUT (sorted):" "$actual" "        🗝 EXPECTED OUTPUT (sorted):" "$expected"
}

for n in 1 2 3 4 5 6 7 8 9 10; do
    check $n
done

rm -f ref stud .dev
touch passed
