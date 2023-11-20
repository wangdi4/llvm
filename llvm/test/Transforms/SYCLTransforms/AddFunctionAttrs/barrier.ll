; Checks the barrier / work_group_barrier / intel_device_barrier decls and calls have
; convergent, "kernel-call-once", "kernel-convergent-call" attributes.

; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

declare void @_Z7barrierj(i32)
declare void @_Z18work_group_barrierj(i32)
declare void @_Z18work_group_barrierj12memory_scope(i32)
declare void @_Z20intel_device_barrierj12memory_scope(i32, i32)

; CHECK: declare void @_Z7barrierj(i32) #0
; CHECK: declare void @_Z18work_group_barrierj(i32) #0
; CHECK: declare void @_Z18work_group_barrierj12memory_scope(i32) #0
; CHECK-LABEL: @foo
; CHECK-SAME: #0
define void @foo() {
entry:
; CHECK: call void @_Z7barrierj
; CHECK-SAME: #0
  call void @_Z7barrierj(i32 1)
; CHECK: call void @_Z18work_group_barrierj
; CHECK-SAME: #0
  call void @_Z18work_group_barrierj(i32 2)
; CHECK: call void @_Z18work_group_barrierj12memory_scope
; CHECK-SAME: #0
  call void @_Z18work_group_barrierj12memory_scope(i32 2)
; CHECK: call void @_Z20intel_device_barrierj12memory_scope
; CHECK-SAME: #0
  call void @_Z20intel_device_barrierj12memory_scope(i32 noundef 2, i32 noundef 2)
  ret void
}

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1

; DEBUGIFY-NOT: WARNING
