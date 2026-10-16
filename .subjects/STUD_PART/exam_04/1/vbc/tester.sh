#!/bin/bash
# Runs from the repo root. Compares the student's vbc with the reference
# (stdout and exit status) on each expression.
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

gcc -Wall -Wextra -Werror -o ref vbc.c || fail "reference did not compile"
gcc -Wall -Wextra -Werror -o stud ../../rendu/vbc/*.c 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(./ref "$@" 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[0]}]")
    actual=$(timeout 5 ./stud "$@" 2>/dev/null | cat -e; echo "[exit ${PIPESTATUS[0]}]")
    if [ "$expected" != "$actual" ]; then
        case "$actual" in *"[exit 124]") actual="$actual   ❌ TIMEOUT" ;; esac
        fail "        💻 TEST" "./vbc '$*'" \
            "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected"
    fi
}

check '1'
check '2+3'
check '3*4+5'
check '3+4*5'
check '(3+4)*5'
check '(((((2+2)*2+2)*2+2)*2+2)*2+2)*2'
check '1+'
check '1+2)'
check '1+2+3+4+5'
check '2*4+9+3+2*1+5+1+6+6*1*1+8*0+0+5+0*4*9*5*8+9*7+5*1+3+1+4*5*7*3+0*3+4*8+8+8+4*0*5*3+5+4+5*7+9+6*6+7+9*2*6*9+2+1*3*7*1*1*5+1+2+7+4+3*4*2+0+4*4*2*2+6+7*5+9+0+8*4+6*7+5+4*4+2+5*5+1+6+3*5*9*9+7*4*3+7+4*9+3+0+1*8+1+2*9*4*5*1+0*1*9+5*3*5+9*6+5*4+5+5*8*6*4*9*2+0+0+1*5*3+6*8*0+0+2*3+7*5*6+8+6*6+9+3+7+0*0+5+2*8+2*7*2+3+9*1*4*8*7*9+2*0+1*6*4*2+8*8*3*1+8+2*4+8*3+8*3+9*5+2*3+9*5*6*4+3*6*6+7+4*8+0+2+9*8*0*6*8*1*2*7+0*5+6*5+0*2+7+2+3+8*7+6+1*3+5+4*5*4*6*1+4*7+9*0+4+9*8+7+5+6+2+6+1+1+1*6*0*9+7+6*2+4*4+1*6*2*9+3+0+0*1*8+4+6*2+6+2*7+7+0*9+6+2*1+6*5*2*3*5*2*6*4+2*9*2*4*5*2*2*3+8+8*3*2*3+0*5+9*6+8+3*1+6*9+8+9*2*0+2'
check '(1)'
check '(((((((3)))))))'
check '(1+2)*3'
check '((6*6+7+5+8)*(1+0+4*8+7)+2)+4*(1+2)'
check '((1+3)*12+(3*(2+6))'
check ''
check '()'
check '2**3'
check '(1+2'
check '1+2*'
check '*1'
check ')'
check 'a'
check '1 + 2'
check '9*9*9*9*9'

rm -f ref stud .dev
touch passed
