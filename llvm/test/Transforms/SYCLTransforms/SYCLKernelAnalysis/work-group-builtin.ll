; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: SYCLKernelAnalysisPass
; CHECK: Kernel <kernel_contains_wg_all>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_any>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_broadcastij>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_broadcastijj>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_broadcastijjj>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_reduce_add>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_reduce_min>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_scan_exclusive_add>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_scan_exclusive_min>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_scan_inclusive_add>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_contains_wg_scan_inclusive_min>:
; CHECK-NEXT: NoBarrierPath=0

define void @kernel_contains_wg_all() {
entry:
  %0 = call spir_func i32 @_Z14work_group_alli(i32 1)
  ret void
}

define void @kernel_contains_wg_any() {
entry:
  %0 = call spir_func i32 @_Z14work_group_anyi(i32 1)
  ret void
}

define void @kernel_contains_wg_broadcastij() {
entry:
  %0 = call spir_func i32 @_Z20work_group_broadcastij(i32 1, i32 1)
  ret void
}

define void @kernel_contains_wg_broadcastijj() {
entry:
  %0 = call spir_func i32 @_Z20work_group_broadcastijj(i32 1, i32 1, i32 1)
  ret void
}

define void @kernel_contains_wg_broadcastijjj() {
entry:
  %0 = call spir_func i32 @_Z20work_group_broadcastijjj(i32 1, i32 1, i32 1, i32 1)
  ret void
}

define void @kernel_contains_wg_reduce_add() {
entry:
  %0 = call spir_func i32 @_Z21work_group_reduce_addi(i32 1)
  ret void
}

define void @kernel_contains_wg_reduce_min() {
entry:
  %0 = call spir_func i32 @_Z21work_group_reduce_minj(i32 1)
  ret void
}

define void @kernel_contains_wg_scan_exclusive_add() {
entry:
  %0 = call spir_func i32 @_Z29work_group_scan_exclusive_addi(i32 1)
  ret void
}

define void @kernel_contains_wg_scan_exclusive_min() {
entry:
  %0 = call spir_func i32 @_Z29work_group_scan_exclusive_minj(i32 1)
  ret void
}

define void @kernel_contains_wg_scan_inclusive_add() {
entry:
  %0 = call spir_func i32 @_Z29work_group_scan_inclusive_addi(i32 1)
  ret void
}

define void @kernel_contains_wg_scan_inclusive_min() {
entry:
  %0 = call spir_func i32 @_Z29work_group_scan_inclusive_minj(i32 1)
  ret void
}

declare spir_func i32 @_Z14work_group_alli(i32) #0
declare spir_func i32 @_Z14work_group_anyi(i32) #0
declare spir_func i32 @_Z20work_group_broadcastij(i32, i32) #0
declare spir_func i32 @_Z20work_group_broadcastijj(i32, i32, i32) #0
declare spir_func i32 @_Z20work_group_broadcastijjj(i32, i32, i32, i32) #0
declare spir_func i32 @_Z21work_group_reduce_addi(i32) #0
declare spir_func i32 @_Z21work_group_reduce_minj(i32) #0
declare spir_func i32 @_Z29work_group_scan_exclusive_addi(i32) #0
declare spir_func i32 @_Z29work_group_scan_exclusive_minj(i32) #0
declare spir_func i32 @_Z29work_group_scan_inclusive_addi(i32) #0
declare spir_func i32 @_Z29work_group_scan_inclusive_minj(i32) #0

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{ptr @kernel_contains_wg_all, ptr @kernel_contains_wg_any, ptr @kernel_contains_wg_broadcastij, ptr @kernel_contains_wg_broadcastijj, ptr @kernel_contains_wg_broadcastijjj, ptr @kernel_contains_wg_reduce_add, ptr @kernel_contains_wg_reduce_min, ptr @kernel_contains_wg_scan_exclusive_add, ptr @kernel_contains_wg_scan_exclusive_min, ptr @kernel_contains_wg_scan_inclusive_add, ptr @kernel_contains_wg_scan_inclusive_min}

; DEBUGIFY-NOT: WARNING
