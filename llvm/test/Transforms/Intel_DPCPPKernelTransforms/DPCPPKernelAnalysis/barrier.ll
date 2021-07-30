; RUN: opt -analyze -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

; CHECK: KernelAnalysis
; CHECK: kernel_contains_barrier no
; CHECK: kernel_not_contains_barrier yes
; CHECK: kernel_call_func_call_barrier no
; CHECK: kernel_call_func_call_func_call_barrier no
; CHECK: kernel_call_func_no_call_barrier yes

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
