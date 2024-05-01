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

CMAKE_TOOLCHAIN_DIR=$CRYPTO_DB_PATH/tools/cmake-toolchains

cd $CRYPTO_DB_PATH && rm -rf build/x86_64 build/x86 && mkdir -p build/x86_64 build/x86

cd $CRYPTO_DB_PATH/build/x86_64
cmake -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ ../..
make

cd $CRYPTO_DB_PATH/build/x86
cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_DIR/gcc-x86.cmake ../..
make

cd $WORK_DIR

echo
echo "Build was successful. Please see 'build/x86_64' and 'build/x86' directories"
echo

exit 0
