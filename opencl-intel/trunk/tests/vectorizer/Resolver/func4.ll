; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define <8 x float> @main(i1 %pred, <4 x float> %param) {
; CHECK: call
  %f = call <8 x float> @multi__param(i1 %pred, <4 x float> %param, <4 x float> %param, <4 x float> %param)
  ret <8 x float> %f
; CHECK: ret
}

declare <8 x float> @multi__param(i1 %pred, <4 x float> %param2, <4 x float> %param1, <4 x float> %param0)


