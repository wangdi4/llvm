; Test that half convert builtins are compiled successfully.

; RUN: SATest -BUILD --config=%S/half-convert.cfg -tsize=0 -cpuarch="corei7" | FileCheck %s

CHECK: Test program was successfully built.
