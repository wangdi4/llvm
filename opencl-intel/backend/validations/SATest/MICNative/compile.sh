#!/bin/sh
SRC=$1
OBJ=$2

# This is valid only for the current engineering ICC build. 
# For the real ICC release you will need lines 10 and 13 to be uncommented and 11,14 - removed. 
# You will need also a support in the CMakeLists.txt file to find ICC setup script and set the DEVICE_INIT_ENV_SCRIPT variable.

# All variables in the form of CMake vars will be replaced by CMake
#source "@DEVICE_INIT_ENV_SCRIPT@" intel64
ICC=/opt/intel/mic/Compiler/bin/icc

#exec $ICC -mmic -c @FINAL_MIC_FLAGS@ "$SRC" -o "$OBJ"  -static-intel
exec $ICC -c @FINAL_MIC_FLAGS@ "$SRC" -o "$OBJ"  -static-intel
