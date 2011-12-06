; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; CHECK: @main
define <4 x float> @main(<4 x float>* %ptr0, <4 x float>* %ptr1, <4 x i1> %pred) {
; CHECK-NOT: @masked
  %f0 = call <4 x float> @masked_load_align0(<4 x i1> %pred, <4 x float>* %ptr0)
; CHECK: ret
  ret <4 x float> %f0
}

declare <4 x float> @masked_load_align0(<4 x i1> %pred, <4 x float>* %ptr0)


