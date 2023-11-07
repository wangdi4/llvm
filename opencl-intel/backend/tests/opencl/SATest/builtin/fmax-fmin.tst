; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7" 2>&1 | FileCheck %s

; CHECK: Test Passed.
