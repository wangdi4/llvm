; RUN: llvm-as %s -o %t.bc
; RUN: opt -micresolve -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; CHECK: @main1
; CHECK: @llvm.mic.load.v16f32
; CHECK: ret

define <16 x float> @main1(<16 x float>* %ptr0, <16 x float>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x float> @masked_load(<16 x i1> %pred, <16 x float>* %ptr0)
  ret <16 x float> %f0
}

declare <16 x float> @masked_load(<16 x i1> %pred, <16 x float>* %ptr0)


; CHECK: @main2
; CHECK: @llvm.mic.load.v16i32
; CHECK: ret
define <16 x i32> @main2(<16 x i32>* %ptr0, <16 x i32>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x i32> @masked_loadi32(<16 x i1> %pred, <16 x i32>* %ptr0)
  ret <16 x i32> %f0
}

declare <16 x i32> @masked_loadi32(<16 x i1> %pred, <16 x i32>* %ptr0)


; CHECK: @main3
; CHECK: @llvm.mic.load.v8f64
; CHECK: ret

define <8 x double> @main3(<8 x double>* %ptr0, <8 x double>* %ptr1, <8 x i1> %pred) {
  %f0 = call <8 x double> @masked_load3(<8 x i1> %pred, <8 x double>* %ptr0)
  ret <8 x double> %f0
}

declare <8 x double> @masked_load3(<8 x i1> %pred, <8 x double>* %ptr0)


; CHECK: @main4
; CHECK: @llvm.mic.load.v8i64
; CHECK: ret
define <8 x i64> @main4(<8 x i64>* %ptr0, <8 x i64>* %ptr1, <8 x i1> %pred) {
  %f0 = call <8 x i64> @masked_load4(<8 x i1> %pred, <8 x i64>* %ptr0)
  ret <8 x i64> %f0
}

declare <8 x i64> @masked_load4(<8 x i1> %pred, <8 x i64>* %ptr0)

