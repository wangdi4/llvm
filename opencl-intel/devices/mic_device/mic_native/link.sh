#!/bin/sh

EXE_NAME=$1
shift
OBJS=$*

# All variables in the form of CMake vars will be replaced by CMake
source "@DEVICE_INIT_ENV_SCRIPT@" intel64
#ICC=/opt/intel/mic/Compiler/bin/icc

# icc $OBJS -o "$EXE_NAME"
exec icc -mmic -Bsymbolic -o "$EXE_NAME" -Wl,--whole-archive $OBJS -Wl,--no-whole-archive -static-intel @FINAL_MIC_LINK_FLAGS@ -Wl,-rpath=\$ORIGIN -Wl,--enable-new-dtags
#exec $ICC                -o "$EXE_NAME" -Wl,--whole-archive $OBJS -Wl,--no-whole-archive @FINAL_MIC_LINK_FLAGS@
