#!/bin/bash

# Function to build shared libraries in a directory
build_shared_libraries() {
    local dir="$1"
    if [ -d "$dir" ]; then
        for script in "$dir"/*/build.sh; do
            if [ -f "$script" ]; then
                echo "Building shared library in $script"
                (cd "$(dirname "$script")" && ./build.sh)
            fi
        done
    fi
}

# Build remakes
echo "Building remakes..."
build_shared_libraries "remakes"

# Build selectors
echo "Building selectors..."
build_shared_libraries "selectors"

# Build loader
echo "Building loader..."
(cd "loader" && ./build.sh)

echo "Build process completed."

