; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define i1 @main(i1 %pred, float %val, float* %ptr) {
; CHECK-NOT: @masked
  call void @masked_store_align0(i1 %pred, float %val, float* %ptr)
  call void @masked_store_align0(i1 %pred, float %val, float* %ptr)
  call void @masked_store_align0(i1 %pred, float %val, float* %ptr)
; CHECK: ret
  ret i1 %pred
}

declare void @masked_store_align0(i1 %pred, float %val, float* %ptr)


