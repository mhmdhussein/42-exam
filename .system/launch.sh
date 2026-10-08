#!/bin/bash
# Checks the environment, then starts the exam binary built by `make`.

RED="\033[31m"
BOLD="\033[1m"
RESET="\033[0m"

fail() {
    printf "$RED$BOLD✗$RESET %s\n" "$1"
    exit 1
}

[ "$(uname -s)" = "Linux" ] || fail "This simulator runs on Linux or WSL only."

for cmd in gcc g++ clang; do
    command -v "$cmd" >/dev/null 2>&1 || fail "$cmd is not installed. Please install it to continue."
done

tmp=$(mktemp)
if ! g++ .system/checkreadline.cpp -lreadline -o "$tmp" 2>/dev/null; then
    rm -f "$tmp"
    fail "readline is missing: install libreadline-dev (Debian/Ubuntu) or readline-devel (Fedora)."
fi
rm -f "$tmp"

[ -x .system/a.out ] || fail "Binary not found, run 'make' first."

if [ -z "$USER" ]; then
    USER=$(whoami)
    export USER
fi

clear
exec ./.system/a.out "$@"
