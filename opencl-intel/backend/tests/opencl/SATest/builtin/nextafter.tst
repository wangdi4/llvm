; RUN: SATest --VAL --config=%s.cfg -cpuarch="corei7-avx" | FileCheck %s

; CHECK: Test Passed.
