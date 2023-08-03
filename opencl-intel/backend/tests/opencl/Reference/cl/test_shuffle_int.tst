; RUN: SATest -VAL -config=%s.cfg -neat=0 --force_ref 2>&1 | FileCheck %s
; CHECK: Test Passed.
