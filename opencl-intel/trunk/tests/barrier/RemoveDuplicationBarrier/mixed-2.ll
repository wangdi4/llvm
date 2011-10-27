; RUN: llvm-as %s -o %t.bc
; RUN: opt -r-d-b -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @fiber.()
  call void @barrier(i32 1)

  ret void
; CHECK-NOT: @barrier
; CHECK-NOT: @fiber.
; CHECK: @dummybarrier.
; CHECK-NOT: @barrier
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @dummybarrier.()
declare void @barrier(i32)
declare void @fiber.()