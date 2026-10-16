#!/bin/sh
# Reference behaviour: ./filter.sh <string>
[ $# -eq 1 ] && [ -n "$1" ] || exit 1
stars=$(printf "%s" "$1" | sed "s/./*/g")
sed "s/$1/$stars/g"
