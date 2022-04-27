; RUN: opt -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s
; RUN: opt -dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: KernelAnalysis
; CHECK: Kernel <kernel_call_func_with_lid>: NoBarrierPath=0
; CHECK: Kernel <kernel_call_func_without_tid>: NoBarrierPath=1

;; CFG for call chain with TID

define void @func_with_lid_level_1(i32 addrspace(1)* %out) {
  %id = call i32 @_Z12get_local_idj(i32 1) 
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @func_with_lid_level_2(i32 addrspace(1)* %out) {
  tail call void @func_with_lid_level_1(i32 addrspace(1)* %out)
  ret void
}

define void @func_with_lid_level_3(i32 addrspace(1)* %out) {
  tail call void @func_with_lid_level_2(i32 addrspace(1)* %out)
  ret void
}

define void @func_with_lid_level_4(i32 addrspace(1)* %out) {
  tail call void @func_with_lid_level_3(i32 addrspace(1)* %out)
  ret void
}

define void @func_with_lid_level_5(i32 addrspace(1)* %out) {
  tail call void @func_with_lid_level_4(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_with_lid(i32 addrspace(1)* %out) {
  tail call void @func_with_lid_level_5(i32 addrspace(1)* %out)
  ret void
}

;; CFG for call chain without TID

define void @func_without_tid_level_1(i32 addrspace(1)* %out) {
  store i32 0, i32 addrspace(1)* %out
  ret void
}

define void @func_without_tid_level_2(i32 addrspace(1)* %out) {
  tail call void @func_without_tid_level_1(i32 addrspace(1)* %out)
  ret void
}

define void @func_without_tid_level_3(i32 addrspace(1)* %out) {
  tail call void @func_without_tid_level_2(i32 addrspace(1)* %out)
  ret void
}

define void @func_without_tid_level_4(i32 addrspace(1)* %out) {
  tail call void @func_without_tid_level_3(i32 addrspace(1)* %out)
  ret void
}

define void @func_without_tid_level_5(i32 addrspace(1)* %out) {
  tail call void @func_without_tid_level_4(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_without_tid(i32 addrspace(1)* %out) {
  tail call void @func_without_tid_level_5(i32 addrspace(1)* %out)
  ret void
}

declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*)* @kernel_call_func_with_lid, void (i32 addrspace(1)*)* @kernel_call_func_without_tid}

; DEBUGIFY-NOT: WARNING
