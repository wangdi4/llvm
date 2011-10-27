; RUN: llvm-as %s -o %t.bc
; RUN: opt -r-d-b -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main() {
  call void @fiber.()
  call void @fiber.()

  ret void
; CHECK: @fiber.()
; CHECK-NOT: @fiber.
; CHECK: ret
}

declare void @fiber.()