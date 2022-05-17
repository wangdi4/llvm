; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that the loop invariant builtin call is hoisted out of loop body.

; CHECK: entryvector_func.preheader
; CHECK: __ocl_svml_{{.*}}_cvti64tofprte1
; CHECK: entryvector_func
; CHECK: Test program was successfully built.
