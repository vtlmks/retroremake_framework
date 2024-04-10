#!/bin/bash

OUT_FILE="remake_blah"

# Build Linux version
gcc -O2 -march=x86-64-v2 -mtune=native -shared -fPIC -o "$OUT_FILE".so remake.c -I../../include

# Build Windows version
x86_64-w64-mingw32-gcc -O2 -march=x86-64-v2 -mtune=native -shared -o "$OUT_FILE".dll remake.c -I../../include

mkdir -p ../../bin/remakes
mv "$OUT_FILE".so ../../bin/remakes
mv "$OUT_FILE".dll ../../bin/remakes
