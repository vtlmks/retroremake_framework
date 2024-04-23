#!/bin/bash

OUT_FILE="template"

# Build Linux version
gcc -O2 -march=x86-64-v2 -mtune=native -shared -fPIC -o "selector_$OUT_FILE.so" selector.c -I../../include
# gcc -g -march=x86-64-v2 -mtune=native -shared -fPIC -o "selector_$OUT_FILE.so" selector.c -I../../include

# Build Windows version
x86_64-w64-mingw32-gcc -O2 -march=x86-64-v2 -mtune=native -shared -o "selector_$OUT_FILE.dll" selector.c -I../../include

[ -e "selector_$OUT_FILE.so" ] && mv "selector_$OUT_FILE.so" ../../bin/remakes
[ -e "selector_$OUT_FILE.dll" ] && mv "selector_$OUT_FILE.dll" ../../bin/remakes

