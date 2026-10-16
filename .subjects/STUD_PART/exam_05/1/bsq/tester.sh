#!/bin/bash
# Runs from the repo root. Runs maps through the reference and the student's
# bsq and compares stdout and stderr ("map error").
cd .system/grading || exit 1
rm -f traceback passed

fail() {
    {
        echo "----------------8<-------------[ START TEST "
        printf '%s\n' "$@"
        echo "----------------8<------------- END TEST ]"
    } > traceback
    rm -f ref stud .dev map_test
    mv traceback ../../traceback
    exit 1
}

gcc -Wall -Wextra -Werror -o ref bsq.c main.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/bsq/*.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# $1 = map content; run as a file argument, or on stdin with $2 = stdin
check() {
    local expected actual how
    printf "$1" > map_test
    if [ "$2" = stdin ]; then
        how="./bsq < map"
        expected=$(./ref < map_test 2>&1 | cat -e)
        actual=$(timeout 10 ./stud < map_test 2>&1 | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    else
        how="./bsq map"
        expected=$(./ref map_test 2>&1 | cat -e)
        actual=$(timeout 10 ./stud map_test 2>&1 | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    fi
    [ "$expected" = "$actual" ] || fail "        💻 TEST: $how" "        MAP:" "$(cat -e map_test)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
}

check "$(cat map.txt)\n"
check "$(cat map.txt)\n" stdin
check '3 . o x\n...\n...\n...\n'
check '3 . o x\no..\n...\n..o\n'
check '4 . o x\n....\n.o..\n....\n...o\n'
check '1 . o x\no\n'
check '1 . o x\n.\n'
check '1 . o x\n.....\n'
check '5 . o x\n.\n.\n.\n.\n.\n'
check '3 - # @\n-#--\n----\n#---\n'
check '2 1 2 3\n1121\n1111\n'
check '6 e o f\neeeeee\neoeeee\neeeeee\neeeeoe\neeeeee\noeeeee\n'
check '2 . o x\noo\noo\n'
check '2 . o x\n...\n..\n'
check '2 . . x\n..\n..\n'
check '2 . o x\n..\n.a\n'
check '3 . o x\n..\n..\n'
check '2 . o x\n..\n..'
check ''
check '0 . o x\n'
check '2 . o\n..\n..\n'
check '2 . o x\n..\n..\n' stdin
check '2 . o x\n..\n.z\n' stdin

rm -f ref stud .dev map_test
touch passed
