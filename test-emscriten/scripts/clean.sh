#!/bin/bash

cd "$(dirname "$0")/.."

echo "Cleaning build artifacts..."

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    [ -d "build" ] && rm -rf build/*
    [ -d "bin" ]   && rm -rf bin/*
    [ -d "lib" ]   && rm -rf lib/*
else
    rm -rf build/* bin/* lib/*
fi

touch build/.gitkeep bin/.gitkeep lib/.gitkeep

rm -f compile_commands.json

echo "Clean complete!"
