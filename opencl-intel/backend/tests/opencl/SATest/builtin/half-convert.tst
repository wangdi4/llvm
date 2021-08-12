; Test that half convert builtins are compiled successfully.

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 -cpuarch="corei7" | FileCheck %s
; RUN: SATest -BUILD --config=%s.cfg -tsize=0 -cpuarch="corei7-avx" | FileCheck %s
; RUN: SATest -BUILD --config=%s.cfg -tsize=0 -cpuarch="core-avx2" | FileCheck %s
; RUN: SATest -BUILD --config=%s.cfg -tsize=0 -cpuarch="skx" | FileCheck %s

CHECK: Test program was successfully built.
