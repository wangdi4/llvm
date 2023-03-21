; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest --VAL --config=%s.cfg -cpuarch="corei7" | FileCheck %s

; CHECK: Test Passed.
