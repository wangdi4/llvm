#!/bin/sh
SRC=$1

# All variables in the form of CMake vars will be replaced by CMake
source "@DEVICE_INIT_ENV_SCRIPT@" intel64
#ICC=/opt/intel/mic/Compiler/bin/icc

exec icc -mmic -M -MG -E @FINAL_MIC_FLAGS@ "$SRC" -static-intel | tr '\\' ' ' | sed -e s/.*:// | tr '\n' ' ' 
