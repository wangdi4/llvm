#!/bin/sh
# Compilation of a C file by ICC Compiler
# All variables in the form of CMake vars will be replaced by CMake:
#     ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
#     ICC_CPU_CL_NATIVE         - executable of ICC compiler
#     BUILD_TYPE_ICC_COMPILER_FLAGS_Debug   - Debug build specific compliler flags 
#     BUILD_TYPE_ICC_COMPILER_FLAGS_Release - Release build specific compliler flags 
# Invocation parameters:
#     $1 - build type
#     $2 - source file
#     $3 - object file

. @ICC_CPU_ENV_SCRIPT_NATIVE@ @ICC_PLATFORM@
if [ "$1" = "Debug" ]
then
    @ICC_CPU_CL_NATIVE@ -c -fPIC -U__MMX__ -fp-model extended -pc80 -prec-div @BUILD_TYPE_ICC_COMPILER_FLAGS_Debug@ -o $3 $2
elif [ "$1" = "Release" ]
then
    @ICC_CPU_CL_NATIVE@ -c -fPIC -U__MMX__ -fp-model extended -pc80 -prec-div @BUILD_TYPE_ICC_COMPILER_FLAGS_Release@ -o $3 $2
else
    echo Unsupported build configuration $1!
    exit 1
fi
