#!/bin/bash

shader2h vertex_shader vertex_shader.glsl
shader2h fragment_shader shader.h fragment_shader.glsl

# Build Linux version
echo "Building linux version."
gcc -O2 -flto -march=x86-64-v2 -mtune=native loader.c -o loader -Iinclude -Llib/linux64 -lasound -l:libglfw3.a -lm

# Build Windows version
echo "Building windows version."
x86_64-w64-mingw32-gcc -O2 -march=x86-64-v2 -mtune=native loader.c -o loader -Iinclude -Llib/win32 -l:libglfw3.a -lwinmm -lntdll -lgdi32 -luser32 -lm

[ -e loader ] && mv loader ../bin
[ -e loader.exe ] && mv loader.exe ../bin

cp loader.h remake.h selector.h ../include
