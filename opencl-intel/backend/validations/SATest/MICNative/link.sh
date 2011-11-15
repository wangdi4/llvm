#!/bin/sh

EXE_NAME=$1
shift
OBJS=$*

# This is valid only for the current engineering ICC build. Update this script to use production icc build.
# You will need also a support in the CMakeLists.txt file to find ICC setup script and set the DEVICE_INIT_ENV_SCRIPT variable.

# All variables in the form of CMake vars will be replaced by CMake
#source "@DEVICE_INIT_ENV_SCRIPT@" intel64
ICC=/opt/intel/mic/Compiler/bin/icc

# icc $OBJS -o "$EXE_NAME"
#exec icc -mmic -shared -Bsymbolic -o "$EXE_NAME" -Wl,-soname="${EXE_NAME##*/}" -Wl,--whole-archive $OBJS -Wl,--no-whole-archive -static-intel @FINAL_MIC_LINK_FLAGS@ -Wl,-rpath=\$ORIGIN -Wl,--enable-new-dtags
exec $ICC -o "$EXE_NAME" -Wl,--whole-archive $OBJS -Wl,--no-whole-archive @FINAL_MIC_LINK_FLAGS@
