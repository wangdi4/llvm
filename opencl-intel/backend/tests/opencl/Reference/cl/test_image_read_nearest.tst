; XFAIL:
; Invalid sampler converter in SATest Reference. Yuri will implement fix
; RUN: SATest -OCL -VAL -config=%s.cfg -ulp_tol=2 -neat=0 --force_ref | FileCheck %s
; CHECK: Test Passed.
