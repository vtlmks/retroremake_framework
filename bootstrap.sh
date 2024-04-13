#!/bin/bash

# The main program to view remakes of old Amiga/Atari/C64 demos/intros/cracktros and similar creations
# is the loader/loader.exe located in the {root}/bin directory.
#
# After compilation, all selectors and remakes are automatically moved to the {root}/bin/remakes directory.
# Additionally, the loader/loader.exe file resides in {root}/bin.
#
[ ! -d "bin/remakes" ] && mkdir -p "bin/remakes"
[ ! -d "remakes" ] && mkdir -p "remakes"
[ ! -d "selectors" ] && mkdir -p "selectors"
[ ! -d "include" ] && mkdir -p "include"

# Function to build GLFW for Linux
build_linux() {
    echo "Building GLFW for Linux..."
    pushd loader/glfw || { echo "Failed to enter GLFW directory, have you checked out the submodules?"; exit 1; }
    mkdir -p build
    cd build || { echo "Failed to enter build directory"; exit 1; }
    cmake .. -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
    make -j"$(nproc)" || { echo "Failed to build GLFW for Linux"; exit 1; }
    mv src/*.a "../../lib/linux64" || { echo "Failed to move GLFW library files"; exit 1; }
    rm -rf build
    popd || { echo "Failed to return to the original directory"; exit 1; }
    echo "GLFW built for Linux successfully."
}

# Function to build GLFW for Windows with MinGW
build_windows() {
    echo "Building GLFW for Windows with MinGW..."
    pushd loader/glfw || { echo "Failed to enter GLFW directory, have you checked out the submodules?"; exit 1; }
    mkdir -p build
    cd build || { echo "Failed to enter build directory"; exit 1; }
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../CMake/x86_64-w64-mingw32.cmake -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
    make -j"$(nproc)" || { echo "Failed to build GLFW for Windows with MinGW"; exit 1; }
    mv src/*.a "../../lib/win32" || { echo "Failed to move GLFW library files"; exit 1; }
    popd || { echo "Failed to return to the original directory"; exit 1; }
    rm -rf build
    echo "GLFW built for Windows with MinGW successfully."
}

# Main script
build_linux
build_windows
