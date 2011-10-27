#!/bin/sh
# Compilation of a C file by ICC Compiler
# All variables in the form of CMake vars will be replaced by CMake:
#     ICC_ENV_SCRIPT_NATIVE - script setting the ICC environment
#     ICC_PLATFORM 			    - parameter to the script above
# 		ICC_CL_NATIVE         - executable of ICC compiler
# 		ICC_COMPILER_PARAMS   - flags for compilation 
#     BUILD_TYPE_ICC_COMPILER_FLAGS_Debug   - Debug build specific compliler flags 
#			BUILD_TYPE_ICC_COMPILER_FLAGS_Release - Release build specific compliler flags 
# Invocation parameters:
#			$1 - build type 
#     $2 - source file
#     $3 - object file
#     $4 - cpp definition for target SIMD arch
#     $5 - additional compilation option(s)


source @ICC_ENV_SCRIPT_NATIVE@ @ICC_PLATFORM@
if [ "$1" == "Debug" ]
then
	@ICC_CL_NATIVE@ @ICC_COMPILER_PARAMS@ @BUILD_TYPE_ICC_COMPILER_FLAGS_Debug@ -D$4 $5 -o $3 $2
elif [ "$1" == "Release" ]
then
	@ICC_CL_NATIVE@ @ICC_COMPILER_PARAMS@ @BUILD_TYPE_ICC_COMPILER_FLAGS_Release@ -D$4 $5 -o $3 $2
else
	echo Unsupported build configuration $1!
	exit 1
fi
