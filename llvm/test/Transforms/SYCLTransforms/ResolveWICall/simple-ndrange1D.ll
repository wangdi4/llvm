; test ndrange_1D() is inlined
; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

declare ptr @_Z10ndrange_1Dm(i64) nounwind readnone
declare ptr @_Z10ndrange_1Dmm(i64, i64) nounwind readnone
declare ptr @_Z10ndrange_1Dmmm(i64, i64, i64) nounwind readnone

define void @enqueue_simple_block(ptr addrspace(1) %res) nounwind !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK: %call1 = call ptr @_Z10ndrange_1Dm(i64 1)
; CHECK: %call2 = call ptr @_Z10ndrange_1Dmm(i64 1, i64 2)
; CHECK: %call3 = call ptr @_Z10ndrange_1Dmmm(i64 1, i64 2, i64 3)
  %call1 = call ptr @_Z10ndrange_1Dm(i64 1) nounwind readnone
  %call2 = call ptr @_Z10ndrange_1Dmm(i64 1, i64 2) nounwind readnone
  %call3 = call ptr @_Z10ndrange_1Dmmm(i64 1, i64 2, i64 3) nounwind readnone
  ret void
}

!opencl.compiler.options = !{!0}
!0 = !{!"-cl-std=CL2.0"}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
