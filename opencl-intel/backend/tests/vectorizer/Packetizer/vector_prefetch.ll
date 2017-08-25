; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -gather-scatter -gather-scatter-prefetch -packetize -packet-size=16 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @vector_prefetch_test(<4 x i32> addrspace(1)* noalias %A, <4 x i32> addrspace(1)* noalias nocapture %B) nounwind {
; CHECK: @vector_prefetch_test
; CHECK: internal.prefetch.gather.v16i32
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %sext = shl i64 %1, 32
  %2 = ashr exact i64 %sext, 32
  %3 = getelementptr inbounds <4 x i32>, <4 x i32> addrspace(1)* %A, i64 %2
  tail call void @_Z8prefetchPU3AS1KDv4_im(<4 x i32> addrspace(1)* %3, i64 1) nounwind
  %4 = load <4 x i32>, <4 x i32> addrspace(1)* %3, align 16
  %5 = getelementptr inbounds <4 x i32>, <4 x i32> addrspace(1)* %B, i64 %2
  store <4 x i32> %4, <4 x i32> addrspace(1)* %5, align 16
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone
declare void @_Z8prefetchPU3AS1KDv4_im(<4 x i32> addrspace(1)*, i64)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}
!opencl.kernel_info = !{!4}

!0 = !{void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*)* @vector_prefetch_test, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{!"-cl-std=CL1.2"}
!3 = !{!"no_barrier_path", i1 true}

!4 = !{void (<4 x i32> addrspace(1)*, <4 x i32> addrspace(1)*)* @vector_prefetch_test, !5}
!5 = !{!3}
