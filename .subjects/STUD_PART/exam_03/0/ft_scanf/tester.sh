#!/bin/bash
# Runs from the repo root. Compares the student's ft_scanf with the reference,
# using main.c (argv[1] picks the format, input comes from stdin).
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

gcc -Wall -Wextra -Werror -o ref main.c ft_scanf.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud main.c ../../rendu/ft_scanf/ft_scanf.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# $1 = stdin, $2 = format number in main.c
check() {
    local expected actual
    expected=$(printf "$1" | ./ref "$2" 2>/dev/null | cat -e)
    actual=$(printf "$1" | timeout 5 ./stud "$2" 2>/dev/null | cat -e; [ ${PIPESTATUS[1]} -eq 124 ] && echo "   ❌ TIMEOUT")
    if [ "$expected" != "$actual" ]; then
        fail "        💻 TEST" "format $(grep -m1 "t == '$2'" -A2 main.c | grep -o '"[^"]*"' | head -1), input: \"$1\"" \
            "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
    fi
}

check '42 hello a\n' 1
check '   -17   word\n\n   z' 1
check 'abc hello world\n' 1
check '' 1
check '12' 1
check 'first second\n' 2
check '  \t first\n\n  second  ' 2
check 'only' 2
check '123 456 789\n' 3
check '+5 -0 2147483647' 3
check '1 2 x' 3
check 'abc\n' 4
check ' a b' 4
check 'a' 4
check '3,4\n' 5
check '3 ,4\n' 5
check 'x=10 q' 6
check 'y=10 q' 6

rm -f ref stud .dev
touch passed
