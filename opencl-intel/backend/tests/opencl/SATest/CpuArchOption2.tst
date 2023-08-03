; RUN: SATest -BUILD -config=%s.cfg -cpuarch="Unsupported_CPU_ARCH" 2>&1 | FileCheck %s
; XFAIL: *

; CHECK: Test Passed.
