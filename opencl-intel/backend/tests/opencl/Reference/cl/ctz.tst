; RUN: SATest -VAL -config=%s.cfg -neat=1 2>&1 | FileCheck %s
; CHECK: Test Passed.
; XFAIL: *
