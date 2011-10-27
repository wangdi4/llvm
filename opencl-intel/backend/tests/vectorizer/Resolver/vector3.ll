; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define <1 x i1> @main(<1 x i1> %pred, <1 x float> %val, <1 x float>* %ptr) {
; CHECK-NOT: @masked
  call void @masked_store(<1 x i1> %pred, <1 x float> %val, <1 x float>* %ptr)
; CHECK: ret
  ret <1 x i1> %pred
}

declare void @masked_store(<1 x i1> %pred, <1 x float> %val, <1 x float>* %ptr)


