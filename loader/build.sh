#!/bin/bash

# Configuration
INCLUDE_PATHS="-Iinclude -I../include"
LIBRARY_PATHS_LINUX="-Llib/linux64"
LIBRARY_PATHS_WINDOWS="-Llib/win32"
COMMON_CFLAGS="-ffunction-sections -fdata-sections"
COMMON_LDFLAGS="-Wl,--gc-sections"
GLFW_STATIC_LIB_LINUX="-l:libglfw3.a"
GLFW_STATIC_LIB_WINDOWS="-l:libglfw3.a"

# Function to setup pkg-config for Linux build
setup_pkg_config() {
    if ! command -v pkg-config &> /dev/null; then
        echo "pkg-config could not be found"
        exit 1
    fi

    # Libraries to check and configure via pkg-config
    local pkg_libs=("x11" "alsa" "gl")
    for lib in "${pkg_libs[@]}"; do
        if pkg-config --exists "$lib"; then
            CFLAGS+=" $(pkg-config --cflags $lib)"
            LDFLAGS+=" $(pkg-config --libs $lib)"
        else
            echo "$lib not found."
            exit 1
        fi
    done
}

# Make sure the compilation stops if any error happens.
set -e

# Pre-build tasks
shader2h vertex_shader vertex_shader.glsl
shader2h fragment_shader shader.h fragment_shader.glsl

setup_pkg_config

# Build Linux version
echo "Building linux version."
ctime -begin .loader_linux
gcc -g -O0 $COMMON_CFLAGS $CFLAGS -o loader loader.c $COMMON_LDFLAGS $LDFLAGS $INCLUDE_PATHS $LIBRARY_PATHS_LINUX $GLFW_STATIC_LIB_LINUX -lm -ldl -pthread
ctime -end .loader_linux $?

# Build Windows version
echo "Building windows version."

ctime -begin .loader_windows
x86_64-w64-mingw32-gcc -O2 $COMMON_CFLAGS loader.c -o loader $INCLUDE_PATHS $LIBRARY_PATHS_WINDOWS $GLFW_STATIC_LIB_WINDOWS -lwinmm -lntdll -lgdi32 -luser32 -lopengl32 -ladvapi32
ctime -end .loader_windows $?

# Post-build tasks
[ -e loader ] && mv loader ../bin
[ -e loader.exe ] && mv loader.exe ../bin

cp loader.h remake.h selector.h ../include
