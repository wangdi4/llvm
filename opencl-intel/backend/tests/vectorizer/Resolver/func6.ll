; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

declare void @maskedf_1__printf(i1 %pred, float* %ptr0)

; CHECK: @main
define void @main(float* %ptr0, float* %ptr1, i1 %pred) {
; CHECK-NOT: @maskedf
  call void @maskedf_1__printf(i1 %pred, float* %ptr0)
  ret void
; CHECK: ret
}



