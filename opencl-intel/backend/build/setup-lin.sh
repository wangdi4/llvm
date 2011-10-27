#!/bin/sh

BUILD=${1-Debug}
DIR=${2-Linux64}
LLVMDIR=${3-`pwd`/../llvm/}

echo "Usage: ./setup-lin.sh [Debug|Release] [Target-dir] [Absolute LLVM Path]"
mkdir -p $DIR
cd $DIR
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD -DLLVM_ENABLE_WERROR:BOOL=ON $LLVMDIR
