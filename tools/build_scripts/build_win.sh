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

cd $CRYPTO_DB_PATH && rm -rf build/win

if [[ ! -z $(type i686-w64-mingw32-gcc 2>&1 | grep "not found") ]] ||
   [[ ! -z $(type i686-w64-mingw32-g++ 2>&1 | grep "not found") ]];
then
    echo "WARNING: 'i686-w64-mingw32-gcc' and/or 'i686-w64-mingw32-g++' packages weren't found."
    echo "         Building for Windows 32-bit platform will be skipped"
else
    mkdir -p build/win/x86
    SKIPPED=0
fi
if [[ ! -z $(type x86_64-w64-mingw32-gcc 2>&1 | grep "not found") ]] ||
   [[ ! -z $(type x86_64-w64-mingw32-g++ 2>&1 | grep "not found") ]];
then
    echo "WARNING: 'x86_64-w64-mingw32-gcc' and/or 'x86_64-w64-mingw32-g++' packages weren't found."
    echo "         Building for Windows 64-bit platform will be skipped"
else
    mkdir -p build/win/x86_64
    SKIPPED=0
fi

# x86
if [ -d $CRYPTO_DB_PATH/build/win/x86 ]; then
    cd $CRYPTO_DB_PATH/build/win/x86
    cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/mingw-w64-i686.cmake -DCRYPTODB_FOR_WINDOWS=ON ../../..
    make
fi

# x86_64
if [ -d $CRYPTO_DB_PATH/build/win/x86_64 ]; then
    cd $CRYPTO_DB_PATH/build/win/x86_64
    cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/mingw-w64-x86_64.cmake -DCRYPTODB_FOR_WINDOWS=ON ../../..
    make
fi

cd $WORK_DIR

if [ $SKIPPED -ne 1 ]; then
    echo
    echo "Build was successful. Please see 'build/win' directory"
    echo
fi

exit 0
