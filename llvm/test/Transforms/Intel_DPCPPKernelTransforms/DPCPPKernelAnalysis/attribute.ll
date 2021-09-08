; RUN: opt -analyze -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

; CHECK: DPCPPKernelAnalysisPass
; CHECK: Kernel <kernel_contains_barrier>: NoBarrierPath=0

define void @kernel_contains_barrier() {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @_Z18work_group_barrierj(i32) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{void ()* @kernel_contains_barrier}
