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

#
# Usage: build_in_docker.sh <platform> where <platform> could be:
# * arm
# * win
# * x86_64
#

if [[ -z $(echo $CRYPTO_DB_PATH) ]]; then
    echo "ERROR: Please execute the following command first:"
    echo "$ source ./setup_env.sh"
    exit 1
fi

if [ -z $1 ]; then
    echo "ERROR: Please specify the platform:"
    echo "       * arm"
    echo "       * win"
    echo "       * x86_64"
    exit 1
fi

if [ "$1" = "x86_64" ]; then
    if [[ -z $(docker image ls cryptodb_x86_64 | grep cryptodb_x86_64) ]]; then
        docker buildx build --platform linux/amd64 -t cryptodb_x86_64 -f $CRYPTO_DB_PATH/tools/build_scripts/Dockerfile.x86_64 $CRYPTO_DB_PATH
        if [[ -z $(docker image ls cryptodb_x86_64 | grep cryptodb_x86_64) ]]; then
            echo "ERROR: Failed to create docker image"
            exit 1
        fi
    fi
    docker run --rm \
           -v $CRYPTO_DB_PATH:/root \
           -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro \
           --user $(id -u):$(id -g) cryptodb_x86_64 /bin/bash -c "export CRYPTO_DB_PATH=/root && /root/tools/build_scripts/build_x86_64.sh"
else
    if [[ -z $(docker image ls cryptodb_arm_win | grep cryptodb_arm_win) ]]; then
        docker buildx build --platform linux/amd64 -t cryptodb_arm_win -f $CRYPTO_DB_PATH/tools/build_scripts/Dockerfile.arm_win $CRYPTO_DB_PATH
        if [[ -z $(docker image ls cryptodb_arm_win | grep cryptodb_arm_win) ]]; then
            echo "ERROR: Failed to create docker image"
            exit 1
        fi
    fi
    docker run --rm \
           -v $CRYPTO_DB_PATH:/root \
           -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro \
           --user $(id -u):$(id -g) cryptodb_arm_win /bin/bash -c "export CRYPTO_DB_PATH=/root && /root/tools/build_scripts/build_$1.sh"
fi
