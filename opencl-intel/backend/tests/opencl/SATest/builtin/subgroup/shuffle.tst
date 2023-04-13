; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest --VAL --config=%s.cfg -cpuarch="corei7" | FileCheck %s
; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest --VAL --config=%s.cfg -cpuarch="corei7-avx" | FileCheck %s
; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest --VAL --config=%s.cfg -cpuarch="core-avx2" | FileCheck %s
; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest --VAL --config=%s.cfg -cpuarch="skx" | FileCheck %s

; CHECK: Test Passed.
