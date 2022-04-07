; XFAIL: *
; RUN: SATest -VAL -config=%s.cfg -neat=0 --force_ref | FileCheck %s
; CHECK: Test Passed.
