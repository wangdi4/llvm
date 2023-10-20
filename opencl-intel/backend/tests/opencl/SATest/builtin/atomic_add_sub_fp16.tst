; RUN: CL_CONFIG_CPU_EXPERIMENTAL_FP16=1 SATest -tsize=8 --VAL --config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Test Passed.
