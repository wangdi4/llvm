; RUN: opt -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

define void @kernel_contains_barrier() #0 {
; CHECK: define void @kernel_contains_barrier() #0 !no_barrier_path ![[NODE:[0-9]+]] {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @_Z18work_group_barrierj(i32) #1

; CHECK: ![[NODE]] = !{i1 false}

attributes #0 = { "sycl-kernel" }
attributes #1 = { convergent }

!sycl.kernels = !{!0}
!0 = !{void ()* @kernel_contains_barrier}
