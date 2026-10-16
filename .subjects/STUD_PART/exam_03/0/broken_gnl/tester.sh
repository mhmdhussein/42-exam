#!/bin/bash
# Runs from the repo root. Builds the reference and the student's
# get_next_line with several BUFFER_SIZEs and compares the lines read.
cd .system/grading || exit 1
rm -f traceback passed

fail() {
    {
        echo "----------------8<-------------[ START TEST "
        printf '%s\n' "$@"
        echo "----------------8<------------- END TEST ]"
    } > traceback
    rm -rf ref stud .dev files
    mv traceback ../../traceback
    exit 1
}

mkdir -p files
printf 'Line 1\nLine 2\nLine 3\n' > files/basic
printf 'Single line\n' > files/single
printf 'Empty\n\n\nLines\n' > files/empty_lines
printf 'No newline at end' > files/no_nl
: > files/empty
printf '\n' > files/only_nl
for i in $(seq 1 200); do printf 'line %d %0*d\n' "$i" $((i % 97)) 0; done > files/long

for bs in 1 3 10 42 9999; do
    gcc -Wall -Wextra -Werror -D BUFFER_SIZE=$bs -o ref main.c get_next_line.c \
        || fail "reference did not compile"
    gcc -Wall -Wextra -Werror -D BUFFER_SIZE=$bs -I ../../rendu/broken_gnl -o stud \
        main.c ../../rendu/broken_gnl/get_next_line.c 2> .dev \
        || fail "        ❌ COMPILATION ERROR (-D BUFFER_SIZE=$bs)" "$(cat .dev)"
    for f in files/*; do
        expected=$(./ref "$f" | cat -e)
        actual=$(timeout 5 ./stud "$f" 2>&1 | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
        [ "$expected" = "$actual" ] || fail "        💻 TEST (BUFFER_SIZE=$bs)" "$(cat -e "$f" | head -5)" \
            "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
    done
    expected=$(./ref < files/basic | cat -e)
    actual=$(timeout 5 ./stud < files/basic 2>&1 | cat -e)
    [ "$expected" = "$actual" ] || fail "        💻 TEST stdin (BUFFER_SIZE=$bs)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
done

rm -rf ref stud .dev files
touch passed
