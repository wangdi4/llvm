#!/bin/sh
SRC=$1
OBJ=$2

# All variables in the form of CMake vars will be replaced by CMake
source "@DEVICE_INIT_ENV_SCRIPT@" intel64
#ICC=/opt/intel/mic/Compiler/bin/icc

exec icc -mmic -c @FINAL_MIC_FLAGS@ "$SRC" -o "$OBJ"  -static-intel
#exec $ICC       -c @FINAL_MIC_FLAGS@ "$SRC" -o "$OBJ"  -static-intel
