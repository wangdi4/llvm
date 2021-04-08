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

define void @kernel_contains_barrier() #0 {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

define void @kernel_not_contains_barrier() #0 {
entry:
  ret void
}

define void @kernel_call_func_call_barrier() #0 {
entry:
  tail call void @func_call_barrier()
  ret void
}

define void @kernel_call_func_call_func_call_barrier() #0 {
entry:
  tail call void @func_call_func_call_barrier()
  ret void
}

define void @kernel_call_func_no_call_barrier() #0 {
entry:
  tail call void @func_no_call_barrier()
  ret void
}

declare void @_Z18work_group_barrierj(i32 %0) #1

attributes #0 = { "sycl_kernel" }
attributes #1 = { convergent }

