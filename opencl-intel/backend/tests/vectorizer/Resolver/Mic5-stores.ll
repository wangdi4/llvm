; RUN: llvm-as %s -o %t.bc
; RUN: opt -micresolve  %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


declare void @masked_store_align8_1(<16 x i1> %pred, <16 x double> %val, <16 x double>* %ptr)
declare void @masked_store_align8_2(<16 x i1> %pred, <16 x i64> %val, <16 x i64>* %ptr)
declare void @masked_store_align4_3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr)
declare void @masked_store_align4_4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr)
declare void @masked_store_align2_5(<16 x i1> %pred, <16 x i16> %val, <16 x i16>* %ptr)
declare void @masked_store_align1_6(<16 x i1> %pred, <16 x i8> %val, <16 x i8>* %ptr)

; CHECK: @main1
; CHECK: @masked_scatter.v16f64
; CHECK: ret
define <16 x i1> @main1(<16 x i1> %pred, <16 x double> %val, <16 x double>* %ptr) {
  call void @masked_store_align8_1(<16 x i1> %pred, <16 x double> %val, <16 x double>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main2
; CHECK: @masked_scatter.v16i64
; CHECK: ret
define <16 x i1> @main2(<16 x i1> %pred, <16 x i64> %val, <16 x i64>* %ptr) {
  call void @masked_store_align8_2(<16 x i1> %pred, <16 x i64> %val, <16 x i64>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main3
; CHECK: @masked_scatter.v16f32
; CHECK: ret
define <16 x i1> @main3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr) {
  call void @masked_store_align4_3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main4
; CHECK: @masked_scatter.v16i32
; CHECK: ret
define <16 x i1> @main4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr) {
  call void @masked_store_align4_4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main5
; CHECK: @masked_scatter.v16i16
; CHECK: ret
define <16 x i1> @main5(<16 x i1> %pred, <16 x i16> %val, <16 x i16>* %ptr) {
  call void @masked_store_align2_5(<16 x i1> %pred, <16 x i16> %val, <16 x i16>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main6
; CHECK: @masked_scatter.v16i8
; CHECK: ret
define <16 x i1> @main6(<16 x i1> %pred, <16 x i8> %val, <16 x i8>* %ptr) {
  call void @masked_store_align1_6(<16 x i1> %pred, <16 x i8> %val, <16 x i8>* %ptr)
  ret <16 x i1> %pred
}
