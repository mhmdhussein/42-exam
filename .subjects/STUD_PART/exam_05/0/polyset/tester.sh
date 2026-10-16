#!/bin/bash
# Runs from the repo root. Builds main.cpp with the reference and with the
# student's files (each in its own directory so headers don't mix) and
# compares the output.
cd .system/grading || exit 1
rm -rf traceback passed build_ref build_stud

GIVEN="main.cpp bag.hpp searchable_bag.hpp array_bag.hpp array_bag.cpp tree_bag.hpp tree_bag.cpp"
EXPECTED="searchable_array_bag.hpp searchable_array_bag.cpp searchable_tree_bag.hpp searchable_tree_bag.cpp set.hpp set.cpp"

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
    cp "../../rendu/polyset/$f" build_stud/ 2>/dev/null || fail "        ❌ MISSING FILE: $f"
done
(cd build_ref && c++ -Wall -Wextra -Werror -std=c++98 -o prog *.cpp) || fail "reference did not compile"
(cd build_stud && c++ -Wall -Wextra -Werror -std=c++98 -o prog *.cpp) 2> .dev \
    || fail "        ❌ COMPILATION ERROR" "$(cat .dev)"

# tree_bag.cpp prints debug lines whose count depends on how set calls it; ignore them.
quiet() {
    grep -Ev "^(create node|duplicate value|destroying value): "
}

check() {
    local expected actual
    expected=$(build_ref/prog "$@" 2>&1 | quiet | cat -e)
    actual=$(timeout 10 build_stud/prog "$@" 2>&1 | quiet | cat -e; [ ${PIPESTATUS[0]} -eq 124 ] && echo "   ❌ TIMEOUT")
    [ "$expected" = "$actual" ] || fail "        💻 TEST" "./a.out $*  (main.cpp)" \
        "        🔎 YOUR OUTPUT:" "$actual" "        🗝 EXPECTED OUTPUT:" "$expected" \
        "        DIFF:" "$(diff <(echo "$expected") <(echo "$actual"))"
}

check 1
check 5 3 8 1 4
check 4 4 2 2 7
check 10 -5 0 23 -42 7 7 15 3
check 1 2 3 4 5 6 7 8 9
check 9 8 7 6 5 4 3 2 1

rm -rf build_ref build_stud .dev
touch passed
