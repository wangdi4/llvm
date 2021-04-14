; RUN: opt -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

; CHECK: "dpcpp-no-barrier-path"="false"

define void @kernel_contains_barrier() #0 {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @_Z18work_group_barrierj(i32) #1

attributes #0 = { "sycl_kernel" }
attributes #1 = { convergent }

