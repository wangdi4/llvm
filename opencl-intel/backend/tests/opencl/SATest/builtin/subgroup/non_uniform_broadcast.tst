; RUN: SATest --VAL --config=%s.cfg 2>&1 | FileCheck %s

; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest --VAL --config=%s.O0.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
