; RUN: SATest -VAL --force_ref -config=%s.cfg -neat=0 2>&1 | FileCheck %s
; CHECK: Test Passed.
