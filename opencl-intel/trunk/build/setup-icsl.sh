#!/bin/sh

BUILD=${1-Debug}
DIR=${2-Linux64}
LLVMDIR=${3-`pwd`/../llvm/}

echo "Usage: ./setup-lin.sh [Debug|Release] [Target-dir] [Absolute LLVM Path]"
mkdir -p $DIR
cd $DIR
cmake -DCMAKE_CXX_COMPILER=/usr/intel/pkgs/gcc/4.3.2/bin/g++ \
      -DCMAKE_C_COMPILER=/usr/intel/pkgs/gcc/4.3.2/bin/gcc   \
      -DCVCC_ASM_PATH=/usr/intel/pkgs/gcc/4.3.2/bin/as  \
      -DCMAKE_BUILD_TYPE:STRING=$BUILD \
      -DLLVM_ENABLE_WERROR:BOOL=OFF $LLVMDIR
