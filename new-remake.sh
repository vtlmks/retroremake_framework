#!/bin/bash

# Check if the script is called with an argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <remake_name>"
    exit 1
fi

# Set variables
TEMPLATE_DIR="templates/remake"
REMOTES_DIR="remakes"
REMAME_NAME="$1"

# Check if the template directory exists
if [ ! -d "$TEMPLATE_DIR" ]; then
    echo "Error: Template directory not found: $TEMPLATE_DIR"
    exit 1
fi

# Check if the remakes directory exists
if [ ! -d "$REMOTES_DIR" ]; then
    echo "Error: Remakes directory not found: $REMOTES_DIR"
    exit 1
fi

# Copy the template directory to the remakes directory
cp -r "$TEMPLATE_DIR" "$REMOTES_DIR/$REMAME_NAME"

# Replace "<name>" with the provided remake name in build.sh
sed -i "s/OUT_FILE=\"template\"/OUT_FILE=\"$REMAME_NAME\"/" "$REMOTES_DIR/$REMAME_NAME/build.sh"

echo "Remake setup complete: $REMAME_NAME"

