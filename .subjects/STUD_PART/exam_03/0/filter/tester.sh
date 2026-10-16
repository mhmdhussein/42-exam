#!/bin/bash
# Runs from the repo root. Compares the student's filter with the reference
# (stdout and exit status) on each case.
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

gcc -Wall -Wextra -Werror -o ref filter.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/filter/filter.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# $1 = command producing stdin, remaining = arguments
check() {
    local feed=$1 expected actual
    shift
    expected=$(eval "$feed" | ./ref "$@" 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[1]}]")
    actual=$(eval "$feed" | timeout 5 ./stud "$@" 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[1]}]")
    if [ "$expected" != "$actual" ]; then
        case "$actual" in *"[exit 124]") actual="$actual   ❌ TIMEOUT" ;; esac
        fail "        💻 TEST" "$feed | ./filter $(printf '"%s" ' "$@")" \
            "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
    fi
}

check "echo 'abcdefaaaabcdeabcabcdabc'" abc
check "echo 'ababcabababc'" ababc
check "echo 'bonjour world bonjour universe'" bonjour
check "echo 'abcabcabc'" abc
check "printf ''" test
check "printf 'line1\ntest line2\ntest line3'" test
check "printf 'aaaaaa\n'" aa
check "printf 'no match here\n'" xyz
check "printf 'this is a very long pattern to test'" "very long pattern"
check "for c in ab ca bc ab c; do printf \$c; sleep 0.05; done" abc
check "seq 1 3000" 12
check "echo hello"
check "echo hello" ""
check "echo hello" a b

rm -f ref stud .dev
touch passed
