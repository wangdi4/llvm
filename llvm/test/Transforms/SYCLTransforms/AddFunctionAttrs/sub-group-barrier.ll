; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

; This is a hand-written sample mimicking output of llvm-spirv.

; CHECK-LABEL: @_Z17sub_group_barrierj12memory_scope
; CHECK-SAME: #0
declare void @_Z17sub_group_barrierj12memory_scope(i32, i32) #0

; CHECK-LABEL: @_Z17sub_group_barrierj
; CHECK-SAME: #0
declare void @_Z17sub_group_barrierj(i32) #0

; CHECK-LABEL: @foo
; CHECK-SAME: #0
define void @foo() #0 {
entry:
; CHECK: call void @_Z17sub_group_barrierj12memory_scope
; CHECK-SAME: #0
  call void @_Z17sub_group_barrierj12memory_scope(i32 1, i32 3) #0
; CHECK: call void @_Z17sub_group_barrierj
; CHECK-SAME: #0
  call void @_Z17sub_group_barrierj(i32 1) #0
  ret void
}

attributes #0 = { noduplicate }

; CHECK-NOT: noduplicate
; CHECK: attributes #0 = { convergent "kernel-call-once" "kernel-convergent-call" }
;; Do not expect other attributes to appear
; CHECK-NOT: #1

; DEBUGIFY-NOT: WARNING
