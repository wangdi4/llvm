; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(i1 %pred, float %val, float* %ptr) {
; CHECK: call
  call void @nothing(i1 %pred)
; CHECK: call
  call void @nothing2(i1 %pred, float %val, float* %ptr)
; CHECK: ret
  ret void
}

declare void @nothing(i1 %pred)
declare void @nothing2(i1 %pred, float %val, float* %ptr)


