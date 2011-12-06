; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(i1 %pred, float %val, float* %ptr) {
; CHECK-NOT: @masked
  call void @masked_store_align0_1234_x_1(i1 %pred, float %val, float* %ptr)
; CHECK: ret
  ret void
}

declare void @masked_store_align0_1234_x_1(i1 %pred, float %val, float* %ptr)


