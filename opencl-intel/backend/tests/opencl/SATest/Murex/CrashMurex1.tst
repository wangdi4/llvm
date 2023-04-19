RUN: SATest -BUILD -config=%s.cfg -cpuarch="corei7" 2>&1 | FileCheck %s

; CHECK: Test program was successfully built.
