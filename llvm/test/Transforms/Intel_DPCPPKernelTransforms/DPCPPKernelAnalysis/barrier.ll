; RUN: opt -analyze -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

; CHECK: KernelAnalysis
; CHECK-DAG: Kernel <kernel_contains_barrier>: NoBarrierPath=0
; CHECK-DAG: Kernel <kernel_not_contains_barrier>: NoBarrierPath=1
; CHECK-DAG: Kernel <kernel_call_func_call_barrier>: NoBarrierPath=0
; CHECK-DAG: Kernel <kernel_call_func_call_func_call_barrier>: NoBarrierPath=0
; CHECK-DAG: Kernel <kernel_call_func_no_call_barrier>: NoBarrierPath=1

define void @func_no_call_barrier() nounwind {
  ret void
}

define void @func_call_barrier() nounwind {
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

define void @func_call_func_call_barrier() nounwind {
  tail call void @func_call_barrier()
  ret void
}

define void @kernel_contains_barrier() {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

define void @kernel_not_contains_barrier() {
entry:
  ret void
}

define void @kernel_call_func_call_barrier() {
entry:
  tail call void @func_call_barrier()
  ret void
}

define void @kernel_call_func_call_func_call_barrier() {
entry:
  tail call void @func_call_func_call_barrier()
  ret void
}

define void @kernel_call_func_no_call_barrier() {
entry:
  tail call void @func_no_call_barrier()
  ret void
}

declare void @_Z18work_group_barrierj(i32 %0) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{void ()* @kernel_contains_barrier, void ()* @kernel_not_contains_barrier, void ()* @kernel_call_func_call_barrier, void ()* @kernel_call_func_call_func_call_barrier, void ()* @kernel_call_func_no_call_barrier}
