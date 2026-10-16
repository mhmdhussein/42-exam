#!/bin/bash
# Runs from the repo root. Draws boards through stdin and compares the
# student's life with the reference after some iterations.
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

gcc -Wall -Wextra -Werror -o ref life.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/life/*.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# $1 = commands on stdin, then width height iterations
check() {
    local expected actual input=$1
    shift
    expected=$(echo "$input" | ./ref "$@" 2>/dev/null | cat -e)
    actual=$(echo "$input" | timeout 10 ./stud "$@" 2>/dev/null | cat -e; [ ${PIPESTATUS[1]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "echo '$input' | ./life $*" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

check 'sdxddssaaww' 5 5 0
check 'sdxssdswdxdddxsaddawxwdxwaa' 10 6 0
check 'dxss' 3 3 0
check 'dxss' 3 3 1
check 'dxss' 3 3 2
check 'dxss' 3 3 3
check 'sdxddssaaww' 5 5 1
check 'sdxddssaaww' 5 5 4
check 'sdxssdswdxdddxsaddawxwdxwaa' 10 6 1
check 'sdxssdswdxdddxsaddawxwdxwaa' 10 6 7
check 'dxdxsxaxdddxsxaxwwwx' 8 8 5
check 'ddxddxsaxdxsxsaxaax' 6 6 20
check 'ddddddxsdxwdxxsaaaxssaxdxdddsx' 12 9 30
check '' 4 3 2
check 'x' 1 1 0
check 'x' 1 1 1
check 'xddddaaaawwwwssss' 3 3 0
check 'xddddaaaawwwwssss' 3 3 1

rm -f ref stud .dev
touch passed
