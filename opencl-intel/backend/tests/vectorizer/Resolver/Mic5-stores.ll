; RUN: llvm-as %s -o %t.bc
; RUN: opt -micresolve -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


declare void @masked_store_align0_1(<8 x i1> %pred, <8 x double> %val, <8 x double>* %ptr)
declare void @masked_store_align0_2(<8 x i1> %pred, <8 x i64> %val, <8 x i64>* %ptr)
declare void @masked_store_align0_3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr)
declare void @masked_store_align0_4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr)

; CHECK: @main1
; CHECK: @llvm.mic.store.v8f64
; CHECK: ret
define <8 x i1> @main1(<8 x i1> %pred, <8 x double> %val, <8 x double>* %ptr) {
  call void @masked_store_align0_1(<8 x i1> %pred, <8 x double> %val, <8 x double>* %ptr)
  ret <8 x i1> %pred
}

; CHECK: @main2
; CHECK: @llvm.mic.store.v8i64
; CHECK: ret
define <8 x i1> @main2(<8 x i1> %pred, <8 x i64> %val, <8 x i64>* %ptr) {
  call void @masked_store_align0_2(<8 x i1> %pred, <8 x i64> %val, <8 x i64>* %ptr)
  ret <8 x i1> %pred
}

; CHECK: @main3
; CHECK: @llvm.mic.store.v16f32
; CHECK: ret
define <16 x i1> @main3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr) {
  call void @masked_store_align0_3(<16 x i1> %pred, <16 x float> %val, <16 x float>* %ptr)
  ret <16 x i1> %pred
}

; CHECK: @main4
; CHECK: @llvm.mic.store.v16i32
; CHECK: ret
define <16 x i1> @main4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr) {
  call void @masked_store_align0_4(<16 x i1> %pred, <16 x i32> %val, <16 x i32>* %ptr)
  ret <16 x i1> %pred
}





