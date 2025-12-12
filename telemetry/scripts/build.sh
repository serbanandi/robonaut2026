#!/bin/bash

BUILD_DIR=build
mkdir -p $BUILD_DIR

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR

echo "Build completed. Binaries are located in $BUILD_DIR/bin"
