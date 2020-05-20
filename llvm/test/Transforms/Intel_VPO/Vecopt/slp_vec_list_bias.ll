; RUN: opt < %s -slp-vectorizer -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake -tti -S | FileCheck %s

; Check no any vectorization triggered with any portion of
; insertelement <8 x i32> instructions that build entire vector.
; Vectorization was triggered by cost bias caused by substructing
; the cost of entire "aggregate build" sequence while
; building vectorizable tree from only a portion of it.

; CHECK-NOT:  extractelement <4 x i32>
; CHECK-NOT:  insertelement <4 x i32>
; CHECK-NOT:  extractelement <2 x i32>
; CHECK-NOT:  insertelement <2 x i32>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test(i32* nocapture %tmp2) {

  %tmp3 = load i32, i32* %tmp2, align 4
  %tmp4 = getelementptr inbounds i32, i32* %tmp2, i64 7
  %tmp5 = load i32, i32* %tmp4, align 4
  %tmp8 = getelementptr inbounds i32, i32* %tmp2, i64 1
  %tmp9 = load i32, i32* %tmp8, align 4
  %tmp10 = getelementptr inbounds i32, i32* %tmp2, i64 6
  %tmp11 = load i32, i32* %tmp10, align 4
  %tmp14 = getelementptr inbounds i32, i32* %tmp2, i64 2
  %tmp15 = load i32, i32* %tmp14, align 4
  %tmp16 = getelementptr inbounds i32, i32* %tmp2, i64 5
  %tmp17 = load i32, i32* %tmp16, align 4
  %tmp20 = getelementptr inbounds i32, i32* %tmp2, i64 3
  %tmp21 = load i32, i32* %tmp20, align 4
  %tmp22 = getelementptr inbounds i32, i32* %tmp2, i64 4
  %tmp23 = load i32, i32* %tmp22, align 4
  %tmp24 = add nsw i32 %tmp23, %tmp21
  %tmp25 = sub nsw i32 %tmp21, %tmp23
  %tmp27 = sub nsw i32 %tmp3, %tmp24
  %tmp28 = add nsw i32 %tmp15, %tmp9
  %tmp29 = sub nsw i32 %tmp9, %tmp15
  %tmp30 = add nsw i32 %tmp27, %tmp29
  %tmp31 = mul nsw i32 %tmp30, 4433
  %tmp32 = mul nsw i32 %tmp27, 6270
  %tmp34 = mul nsw i32 %tmp29, -15137
  %tmp37 = add nsw i32 %tmp25, %tmp11
  %tmp38 = add nsw i32 %tmp17, %tmp5
  %tmp39 = add nsw i32 %tmp37, %tmp38
  %tmp40 = mul nsw i32 %tmp39, 9633
  %tmp41 = mul nsw i32 %tmp25, 2446
  %tmp42 = mul nsw i32 %tmp17, 16819
  %tmp47 = mul nsw i32 %tmp37, -16069
  %tmp48 = mul nsw i32 %tmp38, -3196
  %tmp49 = add nsw i32 %tmp40, %tmp47
  %tmp50 = add nsw i32 %tmp40, %tmp48
  %tmp65 = insertelement <8 x i32> undef, i32 %tmp28, i32 0
  %tmp66 = insertelement <8 x i32> %tmp65, i32 %tmp50, i32 1
  %tmp67 = insertelement <8 x i32> %tmp66, i32 %tmp32, i32 2
  %tmp68 = insertelement <8 x i32> %tmp67, i32 %tmp49, i32 3
  %tmp69 = insertelement <8 x i32> %tmp68, i32 %tmp28, i32 4
  %tmp70 = insertelement <8 x i32> %tmp69, i32 %tmp50, i32 5
  %tmp71 = insertelement <8 x i32> %tmp70, i32 %tmp34, i32 6
  %tmp72 = insertelement <8 x i32> %tmp71, i32 %tmp49, i32 7
  %tmp76 = shl <8 x i32> %tmp72, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %tmp79 = bitcast i32* %tmp2 to <8 x i32>*
  store <8 x i32> %tmp76, <8 x i32>* %tmp79, align 4

  unreachable
}
