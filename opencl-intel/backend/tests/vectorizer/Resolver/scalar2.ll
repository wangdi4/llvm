; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define float @main(float* %ptr0, float* %ptr1, i1 %pred) {
; CHECK-NOT: @masked
  %f0 = call float @masked_load_align0(i1 %pred, float* %ptr0)
; CHECK-NOT: @masked
  %f1 = call float @masked_load_align0(i1 %pred, float* %ptr0)
; CHECK: ret
  ret float %f1
}

declare float @masked_load_align0(i1 %pred, float* %ptr0)


