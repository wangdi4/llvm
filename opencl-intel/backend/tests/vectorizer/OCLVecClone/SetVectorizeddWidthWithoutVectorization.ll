; RUN: %oclopt -ocl-postvect %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @testKernel() !ocl_recommended_vector_length !1 {
; CHECK: define void @testKernel(){{.*}} !vectorized_width ![[#VecWidth:]]
entry:
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void ()* @testKernel}
!1 = !{i32 1}
; CHECK:  ![[#VecWidth]] = !{i32 1}
