; RUN: SATest -tsize=16 --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -tsize=1 --VAL --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
