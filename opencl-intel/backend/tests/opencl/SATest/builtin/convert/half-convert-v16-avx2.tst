; Test that half convert builtins are compiled successfully.

; RUN: SATest -BUILD --config=%S/half-convert-v16.cfg -tsize=0 -cpuarch="core-avx2" 2>&1 | FileCheck %s

CHECK: Test program was successfully built.
