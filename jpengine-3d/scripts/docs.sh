#!/bin/bash

cd "$(dirname "$0")/.."

if ! command -v doxygen >/dev/null 2>&1; then
    echo "doxygen not found."
    echo "Install with: sudo apt install doxygen graphviz"
    exit 1
fi

echo "Generating documentation..."
doxygen Doxyfile

echo "Documentation generated in docs/html/"
echo "Open with: xdg-open docs/html/index.html"
