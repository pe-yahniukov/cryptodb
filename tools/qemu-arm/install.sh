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

if [[ -z $(echo $CRYPTO_DB_PATH) ]]; then
    echo "ERROR: Please execute the following command first:"
    echo "$ source ./setup_env.sh"
    exit 1
fi

build_arch() {
    if [ -z $1 ]; then
        arch="arm"
    else
        arch="$1"
    fi
    cd buildroot-$arch
    if [ "$arch" = "arm" ]; then
        make qemu_arm_versatile_defconfig
    elif [ "$arch" = "armhf" ]; then
        make qemu_arm_vexpress_defconfig
    else
        make qemu_aarch64_virt_defconfig
    fi
    cp ../configs/$arch.config ./.config
    make -j$(nproc)
    cd $CRYPTO_DB_PATH/tools/qemu-arm
}

clean_arch() {
    if [ -z $1 ]; then
        arch="arm"
    else
        arch="$1"
    fi
    cd buildroot-$arch
    make -j$(nproc) clean
    cd $CRYPTO_DB_PATH/tools/qemu-arm
}

WORK_DIR=$(pwd)
ORIG_LD_LIBRARY_PATH=$LD_LIBRARY_PATH

unset LD_LIBRARY_PATH

cd $CRYPTO_DB_PATH/tools/qemu-arm

if [ ! -f buildroot-2023.11.1.tar.gz ]; then
    wget https://buildroot.org/downloads/buildroot-2023.11.1.tar.gz
fi

if [ ! -d buildroot-arm ]; then
    tar xf buildroot-2023.11.1.tar.gz && mv buildroot-2023.11.1 buildroot-arm
fi
if [ ! -d buildroot-armhf ]; then
    tar xf buildroot-2023.11.1.tar.gz && mv buildroot-2023.11.1 buildroot-armhf
fi
if [ ! -d buildroot-arm64 ]; then
    tar xf buildroot-2023.11.1.tar.gz && mv buildroot-2023.11.1 buildroot-arm64
fi

if [ -d buildroot-arm/build/output/images ]; then
    clean_arch arm
fi
build_arch arm

if [ -d buildroot-armhf/build/output/images ]; then
    clean_arch armhf
fi
build_arch armhf

if [ -d buildroot-arm64/build/output/images ]; then
    clean_arch arm64
fi
build_arch arm64

cd $WORK_DIR

export LD_LIBRARY_PATH=$ORIG_LD_LIBRARY_PATH
