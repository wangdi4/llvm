; RUN: SATest --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -cpuarch=core-avx2 --VAL --config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -cpuarch=corei7-avx --VAL --config=%s.cfg 2>&1 | FileCheck %s

; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest --VAL --config=%s.O0.cfg 2>&1 | FileCheck %s
; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest -cpuarch=core-avx2 --VAL --config=%s.O0.cfg 2>&1 | FileCheck %s
; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest -cpuarch=corei7-avx --VAL --config=%s.O0.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
