#!/bin/sh
# Compilation of a C file by ICC MIC Compiler
# All variables in the form of CMake vars will be replaced by CMake:
# 		ICC_MIC_CL              - executable of ICC compiler
# 		FINAL_MIC_FLAGS         - flags for compilation 
# Invocation parameters:
#     $2 - source file
#     $3 - object file
#

source @ICC_MIC_ENV_SCRIPT@ intel64

icpc -mmic -c @FINAL_MIC_FLAGS@ -o $3 $2
