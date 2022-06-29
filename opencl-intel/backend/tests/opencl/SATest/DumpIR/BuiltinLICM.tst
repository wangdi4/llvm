; RUN: SATest -BUILD --config=%s.cfg -cpuarch="skx" --dump-llvm-file - | FileCheck %s

; Check that the loop invariant builtin call is hoisted out of loop body.

; CHECK: entryvector_func.preheader:
; CHECK: call {{.*}} <8 x double> @__ocl_svml_z0_cvti64tofprte8(<8 x i64>
; CHECK: entryvector_func:

; CHECK: scalar_kernel_entry.preheader:
; CHECK: call {{.*}} double @__ocl_svml_z0_cvti64tofprte1(i64
; CHECK: scalar_kernel_entry:

; CHECK: Test program was successfully built.
