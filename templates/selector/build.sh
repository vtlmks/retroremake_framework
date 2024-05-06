#!/bin/bash

OUT_FILE="template"

# Build Linux version
gcc -O2 -shared -fPIC -o "selector_$OUT_FILE.so" selector.c -I../../include
# gcc -g -shared -fPIC -o "selector_$OUT_FILE.so" selector.c -I../../include

# Build Windows version
x86_64-w64-mingw32-gcc -O2 -shared -o "selector_$OUT_FILE.dll" selector.c -I../../include

[ -e "selector_$OUT_FILE.so" ] && mv "selector_$OUT_FILE.so" ../../bin/remakes
[ -e "selector_$OUT_FILE.dll" ] && mv "selector_$OUT_FILE.dll" ../../bin/remakes

