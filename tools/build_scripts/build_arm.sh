#!/bin/bash

#
# MIT License
#
# Copyright 2024 PE Stanislav Yahniukov <pe@yahniukov.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the “Software”), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
# to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

WORK_DIR=$(pwd)

if [[ -z $(echo $CRYPTO_DB_PATH) ]]; then
    echo "ERROR: Please execute the following command first:"
    echo "$ source ./setup_env.sh"
    exit 1
fi

SKIPPED=1
CMAKE_TOOLCHAIN_DIR=$CRYPTO_DB_PATH/tools/cmake-toolchains

cd $CRYPTO_DB_PATH && rm -rf build/arm

if [[ ! -z $(type arm-linux-gnueabi-gcc 2>&1 | grep "not found") ]] ||
   [[ ! -z $(type arm-linux-gnueabi-g++ 2>&1 | grep "not found") ]];
then
    echo "WARNING: 'arm-linux-gnueabi-gcc' and/or 'arm-linux-gnueabi-g++' packages weren't found."
    echo "         Building for ARM32 platform will be skipped"
else
    mkdir -p build/arm/arm
    SKIPPED=0
fi
# if [[ ! -z $(type arm-linux-gnueabihf-gcc 2>&1 | grep "not found") ]] ||
#    [[ ! -z $(type arm-linux-gnueabihf-g++ 2>&1 | grep "not found") ]];
# then
#     echo "WARNING: 'arm-linux-gnueabihf-gcc' and/or 'arm-linux-gnueabihf-g++' packages weren't found."
#     echo "         Building for ARM32 Hard Float platform will be skipped"
# else
#     mkdir -p build/arm/armhf
#     SKIPPED=0
# fi
if [[ ! -z $(type aarch64-linux-gnu-gcc 2>&1 | grep "not found") ]] ||
   [[ ! -z $(type aarch64-linux-gnu-g++ 2>&1 | grep "not found") ]];
then
    echo "WARNING: 'aarch64-linux-gnu-gcc' and/or 'aarch64-linux-gnu-g++' packages weren't found."
    echo "         Building for ARM64 (aarch64) platform will be skipped"
else
    mkdir -p build/arm/arm64
    SKIPPED=0
fi

# arm
if [ -d $CRYPTO_DB_PATH/build/arm/arm ]; then
    cd $CRYPTO_DB_PATH/build/arm/arm
    cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/gcc-arm.cmake ../../..
    make
fi

# armhf
# if [ -d $CRYPTO_DB_PATH/build/arm/armhf ]; then
#     cd $CRYPTO_DB_PATH/build/arm/armhf
#     cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/gcc-armhf.cmake ../../..
#     make
# fi

# arm64
if [ -d $CRYPTO_DB_PATH/build/arm/arm64 ]; then
    cd $CRYPTO_DB_PATH/build/arm/arm64
    cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/gcc-arm64.cmake ../../..
    make
fi

cd $WORK_DIR

if [ $SKIPPED -ne 1 ]; then
    echo
    echo "Build was successful. Please see 'build/arm' directory"
    echo
fi

exit 0
