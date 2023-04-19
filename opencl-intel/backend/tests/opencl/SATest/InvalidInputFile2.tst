; RUN: SATest -VAL -config=%s.cfg -force_ref  2>&1 | FileCheck %s
; XFAIL: *

; CHECK: Test Passed.
