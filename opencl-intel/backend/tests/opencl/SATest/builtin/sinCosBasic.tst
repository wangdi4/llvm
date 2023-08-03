; RUN: SATest -REF --config=%s.cfg -neat=1 2>&1 | FileCheck %s
; XFAIL: *
; CHECK: Reference output generated successfully
