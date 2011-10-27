; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(i1 %pred, float %val, float* %ptr) {
; CHECK: @masked
  call void @maskednothing(i1 %pred, float %val, float* %ptr)
  call void @maskednothing2(i1 %pred, float %val, float* %ptr)
; CHECK: ret
  ret void
}

declare void @maskednothing(i1 %pred, float %val, float* %ptr)
declare void @maskednothing2(i1 %pred, float %val, float* %ptr)


