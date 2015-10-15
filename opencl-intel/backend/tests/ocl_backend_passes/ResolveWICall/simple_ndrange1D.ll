; test ndrange_1D() is inlined
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
%opencl.ndrange_t = type opaque
declare %opencl.ndrange_t* @_Z10ndrange_1Dm(i64) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_1Dmm(i64, i64) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_1Dmmm(i64, i64, i64) nounwind readnone

define void @enqueue_simple_block(i32 addrspace(1)* %res) nounwind {
; CHECK: %call1 = call %opencl.ndrange_t* @_Z10ndrange_1Dm(i64 1)
; CHECK: %call2 = call %opencl.ndrange_t* @_Z10ndrange_1Dmm(i64 1, i64 2)
; CHECK: %call3 = call %opencl.ndrange_t* @_Z10ndrange_1Dmmm(i64 1, i64 2, i64 3)
  %call1 = call %opencl.ndrange_t* @_Z10ndrange_1Dm(i64 1) nounwind readnone
  %call2 = call %opencl.ndrange_t* @_Z10ndrange_1Dmm(i64 1, i64 2) nounwind readnone
  %call3 = call %opencl.ndrange_t* @_Z10ndrange_1Dmmm(i64 1, i64 2, i64 3) nounwind readnone
  ret void
}

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
