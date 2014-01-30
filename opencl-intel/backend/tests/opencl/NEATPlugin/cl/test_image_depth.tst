; RUN: SATest -OCL -VAL -config=%s.cfg -neat=1 --force_ref | FileCheck %s
; XFAIL:
; XCHECK: Test Passed.