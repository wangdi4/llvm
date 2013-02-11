; RUN: SATest -OCL -VAL -config=%s.cfg -neat=1 --force_ref --fma-neat | FileCheck %s
; CHECK: Test Passed.
