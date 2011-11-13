#!/bin/sh
SRC=$1
OBJ=$2

exec @ICC_CL@ -c @FINAL_MIC_FLAGS@ "$SRC" -o "$OBJ" #-static-intel
