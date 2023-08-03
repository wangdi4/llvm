; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

declare void @__builtin_IB_kmp_acquire_lock(ptr addrspace(1)) #0
declare void @__builtin_IB_kmp_release_lock(ptr addrspace(1)) #0

; CHECK-LABEL: @foo
; CHECK-SAME: #0
define void @foo(ptr addrspace(1) %lock) #0 !kernel_arg_base_type !0 !arg_type_null_val !1 {
; CHECK: call void @__builtin_IB_kmp_acquire_lock
; CHECK-SAME: #0
  call void @__builtin_IB_kmp_acquire_lock(ptr addrspace(1) %lock) #0
; CHECK: call void @__builtin_IB_kmp_release_lock
; CHECK-SAME: #0
  call void @__builtin_IB_kmp_release_lock(ptr addrspace(1) %lock) #0
  ret void
}

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1
attributes #0 = { convergent nounwind }

!0 = !{!"int*"}
!1 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
