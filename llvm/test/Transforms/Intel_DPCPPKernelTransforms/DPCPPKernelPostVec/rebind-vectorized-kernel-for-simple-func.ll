; RUN: opt -passes=dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @testKernel() local_unnamed_addr #0 !recommended_vector_length !1 {
; CHECK: define void @testKernel() local_unnamed_addr #{{[0-9]}} !vectorized_kernel ![[#VecKernel1:]] !vectorized_width ![[#VecWidth1:]]
entry:
  ret void
}

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16_testKernel() local_unnamed_addr #0 !recommended_vector_length !1 {
; CHECK: define void @_ZGVeN16_testKernel() local_unnamed_addr #{{[0-9]}} !vectorized_width ![[#VecWidth2:]] !scalar_kernel ![[#ScaKernel2:]]
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind "vector-variants"="_ZGVeN16_testKernel" }

!sycl.kernels = !{!0}

!0 = !{void ()* @testKernel}
!1 = !{i32 16}
; CHECK-DAG:  ![[#ScaKernel2]] = !{void ()* @testKernel}
; CHECK-DAG:  ![[#VecKernel1]] = !{void ()* @_ZGVeN16_testKernel}
; CHECK-DAG:  ![[#VecWidth1]] = !{i32 1}
; CHECK-DAG:  ![[#VecWidth2]] = !{i32 16}

; DEBUGIFY-NOT: WARNING
