; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define <4 x i1> @main(<4 x i1> %pred, <4 x float> %val, <4 x float>* %ptr) {
; CHECK-NOT: @masked
  call void @masked_store_align0(<4 x i1> %pred, <4 x float> %val, <4 x float>* %ptr)
; CHECK: ret
  ret <4 x i1> %pred
}

declare void @masked_store_align0(<4 x i1> %pred, <4 x float> %val, <4 x float>* %ptr)


