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

if [[ -z $(git diff --cached --name-only --diff-filter=ACM 2>&1 | grep "cryptodb") ]]; then
    if [[ -z $(git diff --cached --name-only --diff-filter=ACM 2>&1 | grep "test/test") ]]; then
        exit 0
    fi
fi

if [[ ! -z $(type cppcheck 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'cppcheck' package wasn't found. Please install this package."
    exit 1
fi
if [[ ! -z $(type valgrind 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'valgrind' package wasn't found. Please install this package."
    exit 1
fi

echo "Pre-commit hook: Perform static analysis"
if [[ ! -z $(cppcheck $CRYPTO_DB_PATH/cryptodb.h $CRYPTO_DB_PATH/cryptodb.c $CRYPTO_DB_PATH/test/test.c 2>&1 | grep error) ]]; then
    echo "ERROR: Source code static analysis was failed"
    exit 1
fi
if [[ ! -z $(cppcheck $CRYPTO_DB_PATH/cryptodb.h $CRYPTO_DB_PATH/cryptodb.c $CRYPTO_DB_PATH/test/test.c 2>&1 | grep warning) ]]; then
    echo "ERROR: Source code static analysis was failed"
    exit 1
fi
if [[ ! -z $(cppcheck $CRYPTO_DB_PATH/cryptodb.hpp $CRYPTO_DB_PATH/cryptodb.cpp $CRYPTO_DB_PATH/test/test.cpp 2>&1 | grep error) ]]; then
    echo "ERROR: Source code static analysis was failed"
    exit 1
fi
if [[ ! -z $(cppcheck $CRYPTO_DB_PATH/cryptodb.hpp $CRYPTO_DB_PATH/cryptodb.cpp $CRYPTO_DB_PATH/test/test.cpp 2>&1 | grep warning) ]]; then
    echo "ERROR: Source code static analysis was failed"
    exit 1
fi

echo "Pre-commit hook: Perform rebuilding"
cd $CRYPTO_DB_PATH && rm -rf build && mkdir build && cd build
cmake ..  > /dev/null 2>&1
make > /dev/null 2>&1
cd $WORK_DIR
if [ ! -f $CRYPTO_DB_PATH/build/test/cryptodb_test ]; then
    echo "ERROR: Failed to build test"
    exit 1
fi

echo "Pre-commit hook: Perform memory leaks check"
if [[ -z $(valgrind $CRYPTO_DB_PATH/build/test/cryptodb_test 2>&1 | grep "ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)") ]]; then
    echo "ERROR: Memory leaks check was failed"
    exit 1
fi
if [[ -z $(valgrind $CRYPTO_DB_PATH/build/test/cryptodb_cxx_test 2>&1 | grep "ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)") ]]; then
    echo "ERROR: Memory leaks check was failed"
    exit 1
fi

exit 0
