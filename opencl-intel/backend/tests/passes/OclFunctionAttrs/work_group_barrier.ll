; RUN: %oclopt -ocl-syncfunctionattrs -verify -S < %s | FileCheck %s
; CHECK: Function Attrs: convergent

declare void @_Z18work_group_barrierj12memory_scope(i32, i32)

define void @foo() {
entry:
  call void @_Z18work_group_barrierj12memory_scope(i32 1, i32 1)
  ret void
}
