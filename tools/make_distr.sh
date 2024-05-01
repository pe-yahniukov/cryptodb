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

VERSION="1.0.0"

WORK_DIR=$(pwd)

if [[ -z $(echo $CRYPTO_DB_PATH) ]]; then
    echo "ERROR: Please execute the following command first:"
    echo "$ source ./setup_env.sh"
    exit 1
fi

if [[ ! -z $(type tar 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'tar' package wasn't found. Please install this package."
    exit 1
fi
if [[ ! -z $(type zip 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'zip' package wasn't found. Please install this package."
    exit 1
fi
if [[ ! -z $(type docker 2>&1 | grep "not found") ]]; then
    echo "ERROR: 'docker' package wasn't found. Please install this package."
    exit 1
fi

BUILD_PATH=$CRYPTO_DB_PATH/build

if [ ! -d $BUILD_PATH ]; then
    $CRYPTO_DB_PATH/tools/build_scripts/build_all_in_docker.sh
fi

rm -rf $CRYPTO_DB_PATH/distr && mkdir $CRYPTO_DB_PATH/distr && cd $CRYPTO_DB_PATH/distr

if [ -d $BUILD_PATH/arm ]; then
    if [ -d $BUILD_PATH/arm/arm ]; then
        mkdir -p arm
        cp $BUILD_PATH/arm/arm/libcryptodb.so arm
        cp $BUILD_PATH/arm/arm/libcryptodbcxx.so arm
        cp $BUILD_PATH/arm/arm/external/leveldb/libleveldb.so arm/libleveldb.so.1
    fi
    # if [ -d $BUILD_PATH/arm/armhf ]; then
    #     mkdir -p armhf
    #     cp $BUILD_PATH/arm/armhf/libcryptodb.so armhf
    #     cp $BUILD_PATH/arm/armhf/libcryptodbcxx.so armhf
    #     cp $BUILD_PATH/arm/armhf/external/leveldb/libleveldb.so armhf/libleveldb.so.1
    # fi
    if [ -d $BUILD_PATH/arm/arm64 ]; then
        mkdir -p arm64
        cp $BUILD_PATH/arm/arm64/libcryptodb.so arm64
        cp $BUILD_PATH/arm/arm64/libcryptodbcxx.so arm64
        cp $BUILD_PATH/arm/arm64/external/leveldb/libleveldb.so arm64/libleveldb.so.1
    fi
fi
if [ -d $BUILD_PATH/win ]; then
    if [ -d $BUILD_PATH/win/x86 ]; then
        mkdir -p win32
        cp $BUILD_PATH/win/x86/libcryptodb.dll win32
        cp $BUILD_PATH/win/x86/libcryptodbcxx.dll win32
        cp $BUILD_PATH/win/x86/external/leveldb.windows/libleveldb.dll win32
        cp $CRYPTO_DB_PATH/tools/build_scripts/libwinpthread-1.dll.x86 win32/libwinpthread-1.dll
    fi
    if [ -d $BUILD_PATH/win/x86_64 ]; then
        mkdir -p win64
        cp $BUILD_PATH/win/x86_64/libcryptodb.dll win64
        cp $BUILD_PATH/win/x86_64/libcryptodbcxx.dll win64
        cp $BUILD_PATH/win/x86_64/external/leveldb.windows/libleveldb.dll win64
        cp $CRYPTO_DB_PATH/tools/build_scripts/libwinpthread-1.dll.x86_64 win64/libwinpthread-1.dll
    fi
fi
if [ -d $BUILD_PATH/x86_64 ]; then
    mkdir -p x86_64
    cp $BUILD_PATH/x86_64/libcryptodb.so x86_64
    cp $BUILD_PATH/x86_64/libcryptodbcxx.so x86_64
    cp $BUILD_PATH/x86_64/external/leveldb/libleveldb.so x86_64/libleveldb.so.1
fi
if [ -d $BUILD_PATH/x86 ]; then
    mkdir -p x86
    cp $BUILD_PATH/x86/libcryptodb.so x86
    cp $BUILD_PATH/x86/libcryptodbcxx.so x86
    cp $BUILD_PATH/x86/external/leveldb/libleveldb.so x86/libleveldb.so.1
fi
if [ -d $BUILD_PATH/android ]; then
    if [ -d $BUILD_PATH/android/arm64-v8a ]; then
        mkdir -p android/arm64-v8a
        cp $BUILD_PATH/android/arm64-v8a/libcryptodb.so android/arm64-v8a
        cp $BUILD_PATH/android/arm64-v8a/libcryptodbcxx.so android/arm64-v8a
        cp $BUILD_PATH/android/arm64-v8a/external/leveldb/libleveldb.so android/arm64-v8a
    fi
    if [ -d $BUILD_PATH/android/armeabi-v7a ]; then
        mkdir -p android/armeabi-v7a
        cp $BUILD_PATH/android/armeabi-v7a/libcryptodb.so android/armeabi-v7a
        cp $BUILD_PATH/android/armeabi-v7a/libcryptodbcxx.so android/armeabi-v7a
        cp $BUILD_PATH/android/armeabi-v7a/external/leveldb/libleveldb.so android/armeabi-v7a
    fi
    if [ -d $BUILD_PATH/android/x86 ]; then
        mkdir -p android/x86
        cp $BUILD_PATH/android/x86/libcryptodb.so android/x86
        cp $BUILD_PATH/android/x86/libcryptodbcxx.so android/x86
        cp $BUILD_PATH/android/x86/external/leveldb/libleveldb.so android/x86
    fi
    if [ -d $BUILD_PATH/android/x86_64 ]; then
        mkdir -p android/x86_64
        cp $BUILD_PATH/android/x86_64/libcryptodb.so android/x86_64
        cp $BUILD_PATH/android/x86_64/libcryptodbcxx.so android/x86_64
        cp $BUILD_PATH/android/x86_64/external/leveldb/libleveldb.so android/x86_64
    fi
fi

# licenses
cp $CRYPTO_DB_PATH/LICENSE .
cp -r $CRYPTO_DB_PATH/external/licenses .

# archive
mkdir -p cryptodb_${VERSION}
cp LICENSE cryptodb_${VERSION}
cp -r arm cryptodb_${VERSION}
# cp -r armhf cryptodb_${VERSION}
cp -r arm64 cryptodb_${VERSION}
cp -r win32 cryptodb_${VERSION}
cp -r win64 cryptodb_${VERSION}
cp -r x86_64 cryptodb_${VERSION}
cp -r x86 cryptodb_${VERSION}
cp -r android cryptodb_${VERSION}
cp -r licenses cryptodb_${VERSION}
tar cfJ cryptodb_${VERSION}.tar.xz cryptodb_${VERSION}
zip -rq cryptodb_${VERSION}.zip cryptodb_${VERSION}
rm -rf cryptodb_${VERSION}
cd $WORK_DIR

echo "Done"
