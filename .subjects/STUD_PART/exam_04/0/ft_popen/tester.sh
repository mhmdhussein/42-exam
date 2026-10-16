#!/bin/bash
# Runs from the repo root. Runs commands through the reference and the
# student's ft_popen (see main.c) and compares the output, fd leaks included.
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

gcc -Wall -Wextra -Werror -o ref main.c ft_popen.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud main.c ../../rendu/ft_popen/ft_popen.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(./ref "$@" < /dev/null 2>/dev/null | cat -e)
    actual=$(timeout 10 ./stud "$@" < /dev/null 2>/dev/null | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./a.out $(printf "'%s' " "$@")  (see main.c)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

check r echo hello
check r ls main.c ft_popen.c
check r seq 1 5000
check r printf "a\nb\nc"
check r sh -c "echo out; echo err >&2"
check r nonexistent_command_xyz
check w cat
check w grep o
check w wc -l
check w sed s/l/L/g
check chain
check errors

rm -f ref stud .dev
touch passed
