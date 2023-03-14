; RUN: SATest -pass-manager-type=lto --VAL --config=%s.cfg | FileCheck %s
; RUN: SATest -pass-manager-type=ocl --VAL --config=%s.cfg | FileCheck %s

; CHECK: Test Passed.
