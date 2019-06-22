#!/usr/bin/env bash

# Expects to be run from the project root directory 
# ie. the directory where the src directory is (and
# other project directories are) and where the 
# CMakeLists.txt is and where the build.sh script is.

if [ $# -eq 1 ] && [ $1 == "release" ]; then
    # Go into the build directory
    mkdir -p release-build
    cd release-build

    # Compile
    cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && make

    # Run the executable if it compiled
    if [ $? -eq 0 ]; then
        exec=`find . -type f -perm +111 -maxdepth 1 | cut -c 3-`
        len=${#exec}
        printf '\n\e[0;32m'
        seq -s'#' $(($len+28)) | tr -d '[:digit:]'
        printf '\n# Running \e[0;31m%s\e[0;32m in \e[0;31mrelease\e[0;32m mode #\n' $exec
        seq -s'#' $(($len+28)) | tr -d '[:digit:]'
        printf '\e[0m\n\n'
        "./$exec"
    fi
elif [ $# -eq 1 ] && [ $1 == "debug-build-only" ]; then
    # Go into the build directory
    mkdir -p release-build
    cd release-build

    # Compile
    cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && make
else
    # Go into the build directory
    mkdir -p debug-build
    cd debug-build

    # Compile    
    cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && make

    # Run the executable
    if [ $? -eq 0 ]; then
        exec=`find . -type f -perm +111 -maxdepth 1 | cut -c 3-`
        len=${#exec}
        printf '\n\e[0;32m'
        seq -s'#' $(($len+26)) | tr -d '[:digit:]'
        printf '\n# Running \e[0;33m%s\e[0;32m in \e[0;33mdebug\e[0;32m mode #\n' $exec
        seq -s'#' $(($len+26)) | tr -d '[:digit:]'
        printf '\e[0m\n\n'
        "./$exec"
    fi
fi