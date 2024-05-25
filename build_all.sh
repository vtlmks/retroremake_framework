#!/bin/bash

# Function to build shared libraries in a directory
build_shared_libraries() {
    local dir="$1"
    if [ -d "$dir" ]; then
        for script in "$dir"/*/build.sh; do
            if [ -f "$script" ]; then
                dir_name=$(basename "$(dirname "$script")")
                echo " - Building $dir_name"
                (cd "$(dirname "$script")" && ./build.sh)
            fi
        done
    fi
}

# Build remakes
echo
echo "~~~ Building remakes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
build_shared_libraries "remakes"

# Build selectors
echo
echo "~~~ Building selectors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
build_shared_libraries "selectors"

# Build loader
echo
echo "~~~ Building loader ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
(cd "loader" && ./build.sh)

echo
echo "Build process completed."

