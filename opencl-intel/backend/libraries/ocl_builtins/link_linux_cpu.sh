#!/bin/sh
# Linkage of built-ins shared object (out of BI object files) by ICC linker
# All variables in the form of CMake vars will be replaced by CMake:
#     ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
#     ICC_PLATFORM              - parameter to the script above
# 		ICC_CPU_LD_NATIVE       - executable of ICC linker
# 		ICC_LINKER_PARAMS       - flags for linkage
#     BUILD_TYPE_ICC_LINKER_FLAGS_Debug   - Debug build specific linker flags
#     BUILD_TYPE_ICC_LINKER_FLAGS_Release - Release build specific linker flags
# Invocation parameters:
#			$1 - build type
#     $2 - target shared object
#     $3 - SVML import library (dummy parameter:just for compat with ICC-linker-WIN32)
#     $4 - PDB file (dummy parameter:just for compat with ICC-linker-WIN32)
#     $5 - list of object files


source @ICC_CPU_ENV_SCRIPT_NATIVE@ @ICC_PLATFORM@

if [ "$1" == "Debug" ]
then
	@ICC_CPU_LD_NATIVE@ @ICC_LINKER_PARAMS@ @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ -o $2 $5
elif [ "$1" == "Release" ]
then
	@ICC_CPU_LD_NATIVE@ @ICC_LINKER_PARAMS@ @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ -o $2 $5
else
	echo Unsupported build configuration $1!
	exit 1
fi
