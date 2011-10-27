; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define <32 x float> @main(<32 x float>* %ptr0, <32 x float>* %ptr1, <32 x i1> %pred) {
; CHECK-NOT: @masked
  %f0 = call <32 x float> @masked_load(<32 x i1> %pred, <32 x float>* %ptr0)
; CHECK: ret
  ret <32 x float> %f0
}

declare <32 x float> @masked_load(<32 x i1> %pred, <32 x float>* %ptr0)


