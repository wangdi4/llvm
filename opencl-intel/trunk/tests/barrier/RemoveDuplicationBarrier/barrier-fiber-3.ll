; RUN: llvm-as %s -o %t.bc
; RUN: opt -r-d-b -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main() {
  call void @barrier(i32 3)
  call void @fiber.()

  ret void
; CHECK: @barrier(i32 3)
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @barrier(i32)
declare void @fiber.()