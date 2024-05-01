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

is_sourced() {
    if [ -n "$ZSH_VERSION" ]; then
        case $ZSH_EVAL_CONTEXT in *:file:*) return 0;; esac
    else  # Add additional POSIX-compatible shell names here, if needed.
        case ${0##*/} in dash|-dash|bash|-bash|ksh|-ksh|sh|-sh) return 0;; esac
    fi
    return 1  # NOT sourced.
}

if ! is_sourced; then
    echo "ERROR: Do not launch this script. You need to source it from root directory of the repo:"
    echo "$ source ./setup_env.sh"
    exit 1
fi

if [[ ! -z $(type make 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'make' package wasn't found. Please install this package."
    return 1
fi
if [[ ! -z $(type cmake 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'cmake' package wasn't found. Please install this package."
    return 1
fi

export CRYPTO_DB_PATH=$(realpath $(pwd))

git submodule update --init --recursive

git config diff.ignoreSubmodules dirty

cd $CRYPTO_DB_PATH/external/cJSON           && git checkout v1.7.17 > /dev/null 2>&1
cd $CRYPTO_DB_PATH/external/mbedtls         && git checkout v3.6.0  > /dev/null 2>&1
cd $CRYPTO_DB_PATH/external/leveldb         && git checkout 1.23    > /dev/null 2>&1
cd $CRYPTO_DB_PATH/external/leveldb.windows && git checkout 1.23    > /dev/null 2>&1
cd $CRYPTO_DB_PATH/external/scprng          && git checkout v2.0.2  > /dev/null 2>&1
cd $CRYPTO_DB_PATH/external/crc32c          && git checkout 1.1.2   > /dev/null 2>&1

$CRYPTO_DB_PATH/tools/patches/apply_patches.sh

cp $CRYPTO_DB_PATH/tools/pre-commit-hook.sh $CRYPTO_DB_PATH/.git/hooks/pre-commit

cd $CRYPTO_DB_PATH
