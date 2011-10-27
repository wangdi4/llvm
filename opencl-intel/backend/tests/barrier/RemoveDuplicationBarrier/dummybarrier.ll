; RUN: llvm-as %s -o %t.bc
; RUN: opt -r-d-b -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main() {
  call void @dummybarrier.()
  call void @dummybarrier.()

  ret void
; CHECK: @dummybarrier.()
; CHECK-NOT: @dummybarrier.()
; CHECK: ret
}

declare void @dummybarrier.()