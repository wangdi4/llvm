; RUN: SATest -cpuarch=skx --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -cpuarch=core-avx2 --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -cpuarch=corei7-avx --VAL --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
