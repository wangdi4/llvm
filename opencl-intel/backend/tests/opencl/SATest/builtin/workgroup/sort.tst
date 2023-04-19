; RUN: SATest --VAL --config=%S/sort.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest --VAL --config=%S/sort-key-value.tst.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
