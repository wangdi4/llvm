; XFAIL: *
; RUN: SATest -VAL -config=%s.cfg -ulp_tol=2 -neat=0 --force_ref | FileCheck %s
; CHECK: Test Passed.
