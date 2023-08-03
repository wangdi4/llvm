; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: KernelAnalysis
; CHECK: Kernel <kernel_call_func_with_lid>:
; CHECK-NEXT: NoBarrierPath=1
; CHECK: Kernel <kernel_call_func_without_tid>:
; CHECK-NEXT: NoBarrierPath=1

;; CFG for call chain with TID

define void @func_with_lid_level_1(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  %id = call i32 @_Z12get_local_idj(i32 1)
  %outptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %id
  store i32 0, ptr addrspace(1) %outptr, align 4
  ret void
}

define void @func_with_lid_level_2(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_with_lid_level_1(ptr addrspace(1) %out)
  ret void
}

define void @func_with_lid_level_3(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_with_lid_level_2(ptr addrspace(1) %out)
  ret void
}

define void @func_with_lid_level_4(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_with_lid_level_3(ptr addrspace(1) %out)
  ret void
}

define void @func_with_lid_level_5(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_with_lid_level_4(ptr addrspace(1) %out)
  ret void
}

define void @kernel_call_func_with_lid(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_with_lid_level_5(ptr addrspace(1) %out)
  ret void
}

;; CFG for call chain without TID

define void @func_without_tid_level_1(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  store i32 0, ptr addrspace(1) %out, align 4
  ret void
}

define void @func_without_tid_level_2(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_without_tid_level_1(ptr addrspace(1) %out)
  ret void
}

define void @func_without_tid_level_3(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_without_tid_level_2(ptr addrspace(1) %out)
  ret void
}

define void @func_without_tid_level_4(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_without_tid_level_3(ptr addrspace(1) %out)
  ret void
}

define void @func_without_tid_level_5(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_without_tid_level_4(ptr addrspace(1) %out)
  ret void
}

define void @kernel_call_func_without_tid(ptr addrspace(1) %out) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  tail call void @func_without_tid_level_5(ptr addrspace(1) %out)
  ret void
}

declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @kernel_call_func_with_lid, ptr @kernel_call_func_without_tid}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
