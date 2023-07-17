; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: SYCLKernelAnalysisPass
; CHECK: Kernel <kernel_contains_barrier>:
; CHECK-NEXT: NoBarrierPath=0

define void @kernel_contains_barrier() {
entry:
  tail call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @_Z18work_group_barrierj(i32) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{ptr @kernel_contains_barrier}

; DEBUGIFY-NOT: WARNING
