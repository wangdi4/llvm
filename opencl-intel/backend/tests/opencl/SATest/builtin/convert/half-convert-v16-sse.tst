; Test that half convert builtins are compiled successfully.

; RUN: SATest -BUILD --config=%S/half-convert-v16.cfg -tsize=0 -cpuarch="corei7" | FileCheck %s

CHECK: Test program was successfully built.
