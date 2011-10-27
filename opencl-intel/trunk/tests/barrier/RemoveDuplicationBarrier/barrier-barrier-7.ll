; RUN: llvm-as %s -o %t.bc
; RUN: opt -r-d-b -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main() {
  call void @barrier(i32 3)
  call void @barrier(i32 2)

  ret void
; CHECK-NOT: @barrier
; CHECK: @barrier(i32 2)
; CHECK-NOT: @barrier
; CHECK: ret
}

declare void @barrier(i32)