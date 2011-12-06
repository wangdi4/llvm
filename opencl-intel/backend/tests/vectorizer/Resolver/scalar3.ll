; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(float* %ptr1, i1 %pred, float* %ptr0) {
; CHECK-NOT: @masked
  %f1 = call float @masked_load_align0_2(i1 %pred, float* %ptr0)
; CHECK-NOT: @masked
  %f2 = call float @masked_load_align0_3(i1 %pred, float* %ptr0)
; CHECK-NOT: @masked
  %f3 = call float @masked_load_align0_4(i1 %pred, float* %ptr0)
; CHECK: ret
  ret void
}

declare float @masked_load_align0_2(i1 %pred, float* %ptr0)
declare float @masked_load_align0_3(i1 %pred, float* %ptr0)
declare float @masked_load_align0_4(i1 %pred, float* %ptr0)


