#!/bin/bash

OUT_FILE="selector_blah"

# Build Linux version
gcc -O2 -march=x86-64-v2 -mtune=native -shared -fPIC -o "$OUT_FILE".so selector.c -I../../include

# Build Windows version
x86_64-w64-mingw32-gcc -O2 -march=x86-64-v2 -mtune=native -shared -o "$OUT_FILE".dll selector.c -I../../include

[ -e "$OUT_FILE".so ] && mv "$OUT_FILE".so ../../bin/remakes
[ -e "$OUT_FILE".dll ] && mv "$OUT_FILE".dll ../../bin/remakes

