; RUN: llvm-as %s -o %t.bc
; RUN: opt -micresolve %t.bc -S -o %t1.ll 
; RUN: FileCheck %s --input-file=%t1.ll


; CHECK: @main1
; CHECK: @masked_gather.v16f32
; CHECK: ret

define <16 x float> @main1(<16 x float>* %ptr0, <16 x float>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x float> @masked_load_align4(<16 x i1> %pred, <16 x float>* %ptr0)
  ret <16 x float> %f0
}

declare <16 x float> @masked_load_align4(<16 x i1> %pred, <16 x float>* %ptr0)


; CHECK: @main2
; CHECK: @masked_gather.v16i32
; CHECK: ret
define <16 x i32> @main2(<16 x i32>* %ptr0, <16 x i32>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x i32> @masked_load_align4_i32(<16 x i1> %pred, <16 x i32>* %ptr0)
  ret <16 x i32> %f0
}

declare <16 x i32> @masked_load_align4_i32(<16 x i1> %pred, <16 x i32>* %ptr0)


; CHECK: @main3
; CHECK: @masked_gather.v16f64
; CHECK: ret

define <16 x double> @main3(<16 x double>* %ptr0, <16 x double>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x double> @masked_load_align8_3(<16 x i1> %pred, <16 x double>* %ptr0)
  ret <16 x double> %f0
}

declare <16 x double> @masked_load_align8_3(<16 x i1> %pred, <16 x double>* %ptr0)


; CHECK: @main4
; CHECK: @masked_gather.v16i64
; CHECK: ret
define <16 x i64> @main4(<16 x i64>* %ptr0, <16 x i64>* %ptr1, <16 x i1> %pred) {
  %f0 = call <16 x i64> @masked_load_align8_4(<16 x i1> %pred, <16 x i64>* %ptr0)
  ret <16 x i64> %f0
}

declare <16 x i64> @masked_load_align8_4(<16 x i1> %pred, <16 x i64>* %ptr0)

