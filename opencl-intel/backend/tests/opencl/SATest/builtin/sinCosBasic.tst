; RUN: SATest -REF --config=%s.cfg -neat=1 2>&1 | FileCheck %s
; CHECK: Reference output generated successfully
