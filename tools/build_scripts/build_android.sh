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

if [[ -z $NDK_PATH ]]; then
    export NDK_PATH=~/Android/Sdk/ndk/$(ls ~/Android/Sdk/ndk/ -t | head -1)
    if [ ! -d $NDK_PATH ]; then
        echo "ERROR: Failed to find Android NDK path. Please set it manually if you want to build for Android"
        unset NDK_PATH
        exit 1
    fi
fi

cd $CRYPTO_DB_PATH && rm -rf build/android
mkdir -p build/android/arm64-v8a   \
         build/android/armeabi-v7a \
         build/android/x86         \
         build/android/x86_64

ANDROID_VERSION=21 # Android 5.0 and above

# arm64-v8a
cd $CRYPTO_DB_PATH/build/android/arm64-v8a
cmake                                                                      \
    -DANDROID_ABI=arm64-v8a                                                \
    -DANDROID_ARM_MODE=arm                                                 \
    -DANDROID_PLATFORM=android-${ANDROID_VERSION}                          \
    -DANDROID_TOOLCHAIN=clang                                              \
    -DCMAKE_ASM_FLAGS="--target=aarch64-linux-android${ANDROID_VERSION}"   \
    -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
    ../../..
make

# armeabi-v7a
cd $CRYPTO_DB_PATH/build/android/armeabi-v7a
cmake                                                                      \
    -DANDROID_ABI=armeabi-v7a                                              \
    -DANDROID_ARM_MODE=arm                                                 \
    -DANDROID_PLATFORM=android-${ANDROID_VERSION}                          \
    -DANDROID_TOOLCHAIN=clang                                              \
    -DCMAKE_ASM_FLAGS="--target=arm-linux-androideabi${ANDROID_VERSION}"   \
    -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
    ../../..
make

# x86
cd $CRYPTO_DB_PATH/build/android/x86
cmake                                                                      \
    -DANDROID_ABI=x86                                                      \
    -DANDROID_PLATFORM=android-${ANDROID_VERSION}                          \
    -DANDROID_TOOLCHAIN=clang                                              \
    -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
    ../../..
make

# x86_64
cd $CRYPTO_DB_PATH/build/android/x86_64
cmake                                                                      \
    -DANDROID_ABI=x86_64                                                   \
    -DANDROID_PLATFORM=android-${ANDROID_VERSION}                          \
    -DANDROID_TOOLCHAIN=clang                                              \
    -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
    ../../..
make

cd $WORK_DIR

echo
echo "Build was successful. Please see 'build/android' directory"
echo

exit 0
