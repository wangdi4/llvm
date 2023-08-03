; RUN: SATest -pass-manager-type=lto --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -pass-manager-type=ocl --VAL --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
