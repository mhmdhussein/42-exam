#!/bin/bash
# Runs from the repo root. Builds main.cpp with the reference and with the
# student's files (each in its own directory so headers don't mix) and
# compares the output.
cd .system/grading || exit 1
rm -rf traceback passed build_ref build_stud

GIVEN="main.cpp"
EXPECTED="vect2.hpp vect2.cpp"

fail() {
    {
        echo "----------------8<-------------[ START TEST "
        printf '%s\n' "$@"
        echo "----------------8<------------- END TEST ]"
    } > traceback
    rm -rf build_ref build_stud .dev
    mv traceback ../../traceback
    exit 1
}

mkdir build_ref build_stud
for f in $GIVEN; do cp "$f" build_ref/ && cp "$f" build_stud/; done
for f in $EXPECTED; do
    cp "$f" build_ref/
    cp "../../rendu/vect2/$f" build_stud/ 2>/dev/null || fail "        ❌ MISSING FILE: $f"
done
(cd build_ref && c++ -Wall -Wextra -Werror -std=c++98 -o prog *.cpp) || fail "reference did not compile"
(cd build_stud && c++ -Wall -Wextra -Werror -std=c++98 -o prog *.cpp) 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

check() {
    local expected actual
    expected=$(build_ref/prog "$@" 2>&1 | cat -e)
    actual=$(timeout 10 build_stud/prog "$@" 2>&1 | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./a.out $*  (main.cpp)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected" \
        "        DIFF:" "$(diff <(echo "$expected") <(echo "$actual"))"
}

check

rm -rf build_ref build_stud .dev
touch passed
