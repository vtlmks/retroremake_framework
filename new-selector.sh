#!/bin/bash

# Check if the script is called with an argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <selector_name>"
    exit 1
fi

# Set variables
TEMPLATE_DIR="templates/selector"
SELECTORS_DIR="selectors"
SELECTOR_NAME="$1"

# Check if the template directory exists
if [ ! -d "$TEMPLATE_DIR" ]; then
    echo "Error: Template directory not found: $TEMPLATE_DIR"
    exit 1
fi

# Check if the selectors directory exists
if [ ! -d "$SELECTORS_DIR" ]; then
    echo "Error: Selectors directory not found: $SELECTORS_DIR"
    exit 1
fi

# Copy the template directory to the selectors directory
cp -r "$TEMPLATE_DIR" "$SELECTORS_DIR/$SELECTOR_NAME"

# Replace "<name>" with the provided selector name in build.sh
sed -i "s/OUT_FILE=\"template\"/OUT_FILE=\"$SELECTOR_NAME\"/" "$SELECTORS_DIR/$SELECTOR_NAME/build.sh"

echo "Selector setup complete: $SELECTOR_NAME"

