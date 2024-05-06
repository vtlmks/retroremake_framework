#!/bin/bash

OUT_FILE="template"

# Build Linux version
gcc -O2 -shared -fPIC -o "remake_$OUT_FILE.so" remake.c -I../../include
# gcc -g -shared -fPIC -o "remake_$OUT_FILE.so" remake.c -I../../include

# Build Windows version
x86_64-w64-mingw32-gcc -O2 -shared -o "remake_$OUT_FILE.dll" remake.c -I../../include

[ -e "remake_$OUT_FILE.so" ] && mv "remake_$OUT_FILE.so" ../../bin/remakes
[ -e "remake_$OUT_FILE.dll" ] && mv "remake_$OUT_FILE.dll" ../../bin/remakes

