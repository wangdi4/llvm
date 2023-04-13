; RUN: SATest -tsize=8 --VAL --config=%s.cfg | FileCheck %s
; RUN: SATest -tsize=1 --VAL --config=%s.cfg | FileCheck %s

; CHECK: Test Passed.
