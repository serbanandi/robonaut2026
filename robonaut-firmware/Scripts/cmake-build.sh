#!/bin/bash
BUILD_TYPE=${1:-Debug}

echo "Selected build Type: $BUILD_TYPE"

cmake --preset $BUILD_TYPE
cmake --build --preset $BUILD_TYPE
