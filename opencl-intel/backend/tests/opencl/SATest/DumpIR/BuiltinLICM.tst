; RUN: SATest -BUILD --config=%s.cfg -cpuarch="skx" --tsize=1 --dump-llvm-file - | FileCheck %s

; Check that the loop invariant builtin call is hoisted out of loop body.

; CHECK: wrapper_entry:
; CHECK: = call{{.*}} double @{{_Z14convert_doublel|__ocl_svml_z0_cvti64tofprte1}}(i64
; CHECK: scalar_kernel_entry:

; CHECK: Test program was successfully built.
