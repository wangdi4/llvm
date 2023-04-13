; RUN: SATest --VAL --config=%S/sort.tst.cfg | FileCheck %s
; RUN: SATest --VAL --config=%S/sort-key-value.tst.cfg | FileCheck %s

; CHECK: Test Passed.
