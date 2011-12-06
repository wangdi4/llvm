; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define <32 x i1> @main(<32 x i1> %pred, <32 x float> %val, <32 x float>* %ptr) {
; CHECK-NOT: @masked
  call void @masked_store_align0(<32 x i1> %pred, <32 x float> %val, <32 x float>* %ptr)
; CHECK: ret
  ret <32 x i1> %pred
}

declare void @masked_store_align0(<32 x i1> %pred, <32 x float> %val, <32 x float>* %ptr)


