#!/bin/sh
# ImathLibd DLL build by ICC linker
# All variables in the form of CMake vars will be replaced by CMake:
#   ICC_CPU_ENV_SCRIPT_NATIVE - script setting the ICC environment
#   ICC_PLATFORM 		        - parameter to the script above
#   ICC_LD_NATIVE             - executable of ICC linker
#   BUILD_TYPE_ICC_LINKER_FLAGS_Debug   - Debug build specific linker flags
#   BUILD_TYPE_ICC_LINKER_FLAGS_Release - Release build specific linker flags
# Invocation parameters:
#     $1 - build type
#     $2 - target DLL
#     $3 - PDB file
#     $4 - IMPLIB
#     $5 - list of object files


. @ICC_CPU_ENV_SCRIPT_NATIVE@ @ICC_PLATFORM@

if [ "$1" = "Debug" ]
then
	@ICC_CPU_LD_NATIVE@ -fPIC -static-intel -shared -Wl,-rpath=\\\$ORIGIN @BUILD_TYPE_ICC_LINKER_FLAGS_Debug@ -o $2 $5
elif [ "$1" = "Release" ]
then
	@ICC_CPU_LD_NATIVE@ -fPIC -static-intel -shared -Wl,-rpath=\\\$ORIGIN @BUILD_TYPE_ICC_LINKER_FLAGS_Release@ -o $2 $5
else
	echo Unsupported build configuration $1!
	exit 1
fi
