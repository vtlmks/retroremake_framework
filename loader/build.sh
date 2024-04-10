#!/bin/bash

# These are to make sure pushd/popd does not echo the path to stdout
pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

shader2h vertex_shader vertex_shader.glsl
shader2h fragment_shader shader.h fragment_shader.glsl

# Build Linux version
echo "Building linux version."
time gcc -g -flto -march=x86-64-v2 -mtune=native loader.c -o loader -Iinclude -Llib/linux64 -lglfw3 -lm -lasound

# Build Windows version
#echo "Building windows version."
#time x86_64-w64-mingw32-gcc -O2 -march=x86-64-v2 -mtune=native loader.c -o loader -I. -Iinclude -I.. -Llib/win32 -lglfw3 -lwinmm -lntdll -lgdi32 -luser32 -lm

mkdir -p ../bin/remakes
mkdir -p ../include
mv loader ../bin/
mv loader.exe ../bin/
cp loader.h ../include/
