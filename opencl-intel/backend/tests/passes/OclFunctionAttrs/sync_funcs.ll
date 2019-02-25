; RUN: %oclopt -ocl-syncfunctionattrs -verify -S < %s | FileCheck %s
; CHECK: Function Attrs: convergent

declare void @_Z7barrierj(i64)

define void @foo() {
entry:
  call void @_Z7barrierj(i64 1)
  ret void
}
