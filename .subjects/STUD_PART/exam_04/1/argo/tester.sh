#!/bin/bash
# Runs from the repo root. Feeds JSON files to the reference and the
# student's argo (argo.c includes the given main) and compares the output.
cd .system/grading || exit 1
rm -f traceback passed

fail() {
    {
        echo "----------------8<-------------[ START TEST "
        printf '%s\n' "$@"
        echo "----------------8<------------- END TEST ]"
    } > traceback
    rm -f ref stud .dev input.json
    mv traceback ../../traceback
    exit 1
}

gcc -Wall -Wextra -Werror -o ref argo.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/argo/argo.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# $1 = file content (written as is, no trailing newline)
check() {
    local expected actual
    printf '%s' "$1" > input.json
    expected=$(./ref input.json 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[0]}]")
    actual=$(timeout 5 ./stud input.json 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[0]}]")
    if [ "$expected" != "$actual" ]; then
        case "$actual" in *"[exit 124]") actual="$actual   ❌ TIMEOUT" ;; esac
        fail "        💻 TEST" "echo -n '$1' | ./argo /dev/stdin" \
            "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
    fi
}

check '1'
check '-42'
check '0'
check '"bonjour"'
check '"escape! \" "'
check '"\\"'
check '"a\\\"b"'
check '""'
check '{}'
check '{"tomatoes":42,"potatoes":234}'
check '{"recursion":{"recursion":{"recursion":{"recursion":"recursion"}}}}'
check '{"a":{"b":"c"},"d":-3,"e":{},"f":"g\"h"}'
check '{"":""}'
check '"unfinished string'
check '"unfinished string 2\"'
check '{"no value?":}'
check '{"a":1,}'
check '{"a" :1}'
check '{"a":1 }'
check '{1:2}'
check '{"a"}'
check '{"a":1'
check '{'
check ''
check ' 1'
check 'x'
check '"bad \n escape"'

rm -f ref stud .dev input.json
touch passed
