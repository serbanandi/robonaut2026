cmake_minimum_required(VERSION 3.22)
project(TelemetrySystem C)

# Set C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Find PkgConfig (Standard helper tool on Linux)
find_package(PkgConfig REQUIRED)
# Use PkgConfig to find libzmq
# This will find the include paths and library names automatically
pkg_check_modules(ZMQ REQUIRED libzmq)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/log)
add_subdirectory(src/tel-server)
add_subdirectory(src/tel-cli)