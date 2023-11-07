; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7" 2>&1 | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7-avx" 2>&1 | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="core-avx2" 2>&1 | FileCheck %s
; RUN: SATest --VAL --config=%s.cfg -cpuarch="skx" 2>&1 | FileCheck %s

; CHECK: Test Passed.
