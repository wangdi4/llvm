; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7" | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7-avx" | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="core-avx2" | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="skx" | FileCheck %s

; CHECK: Test Passed.
