; RUN: SATest --VAL --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: 1.000000,2.000000,3.000000,4.000000
; CHECK: 1.00,2.00,3.00,4.00

; CHECK: Test Passed.
