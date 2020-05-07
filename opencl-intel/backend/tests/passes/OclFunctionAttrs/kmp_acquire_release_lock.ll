; RUN: %oclopt -ocl-syncfunctionattrs -verify -S < %s | FileCheck %s

declare void @__builtin_IB_kmp_acquire_lock(i32 addrspace(1)*) #0
declare void @__builtin_IB_kmp_release_lock(i32 addrspace(1)*) #0

; CHECK-LABEL: @foo
; CHECK-SAME: #0
define void @foo(i32 addrspace(1)* %lock) #0 {
; CHECK: call void @__builtin_IB_kmp_acquire_lock
; CHECK-SAME: #0
  call void @__builtin_IB_kmp_acquire_lock(i32 addrspace(1)* %lock) #0
; CHECK: call void @__builtin_IB_kmp_release_lock
; CHECK-SAME: #0
  call void @__builtin_IB_kmp_release_lock(i32 addrspace(1)* %lock) #0
  ret void
}

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1
attributes #0 = { convergent nounwind }
