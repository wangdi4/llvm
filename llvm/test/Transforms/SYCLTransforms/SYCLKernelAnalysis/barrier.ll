; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: KernelAnalysis
; CHECK: Kernel <kernel_contains_barrier>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_not_contains_barrier>:
; CHECK-NEXT: NoBarrierPath=1
; CHECK: Kernel <kernel_call_func_call_barrier>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_call_func_call_func_call_barrier>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_call_func_no_call_barrier>:
; CHECK-NEXT: NoBarrierPath=1
; CHECK: Kernel <kernel_contains_intel_device_barrier>:
; CHECK-NEXT: NoBarrierPath=0

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

define void @func_call_intel_device_barrier() nounwind {
  tail call void @_Z20intel_device_barrierj12memory_scope(i32 noundef 2, i32 noundef 2)
  ret void
}

define void @kernel_contains_intel_device_barrier() {
entry:
  tail call void @func_call_intel_device_barrier()
  ret void
}

declare void @_Z18work_group_barrierj(i32 %0) #0

declare void @_Z20intel_device_barrierj12memory_scope(i32, i32) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{ptr @kernel_contains_barrier, ptr @kernel_not_contains_barrier, ptr @kernel_call_func_call_barrier, ptr @kernel_call_func_call_func_call_barrier, ptr @kernel_call_func_no_call_barrier, ptr @kernel_contains_intel_device_barrier}

; DEBUGIFY-NOT: WARNING
