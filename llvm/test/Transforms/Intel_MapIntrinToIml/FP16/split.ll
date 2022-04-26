; Check split FP16 SVML calls are generated correctly when needed
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sins16
; CHECK: [[ARG1:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG2]])
; CHECK: [[RESULT:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: ret <16 x half> [[RESULT]]

define <16 x half> @test_sins16(<16 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16(<16 x half> %A)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sins32
; CHECK: [[ARG1:%.*]] = shufflevector <32 x half> %A, <32 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x half> %A, <32 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x half> %A, <32 x half> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x half> %A, <32 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT34:%.*]] = shufflevector <8 x half> [[RESULT3]], <8 x half> [[RESULT4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = shufflevector <16 x half> [[RESULT12]], <16 x half> [[RESULT34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: ret <32 x half> [[RESULT]]

define <32 x half> @test_sins32(<32 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <32 x half> @__svml_sins32(<32 x half> %A)
  ret <32 x half> %0
}

; CHECK-LABEL: @test_sins64
; CHECK: [[ARG1:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG2]])
; CHECK: [[ARG3:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG3]])
; CHECK: [[ARG4:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG4]])
; CHECK: [[ARG5:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39>
; CHECK: [[RESULT5:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG5]])
; CHECK: [[ARG6:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47>
; CHECK: [[RESULT6:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG6]])
; CHECK: [[ARG7:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55>
; CHECK: [[RESULT7:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG7]])
; CHECK: [[ARG8:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <8 x i32> <i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT8:%.*]] = call fast svml_cc <8 x half> @__svml_sins8(<8 x half> [[ARG8]])
; CHECK: [[RESULT12:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT34:%.*]] = shufflevector <8 x half> [[RESULT3]], <8 x half> [[RESULT4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT56:%.*]] = shufflevector <8 x half> [[RESULT5]], <8 x half> [[RESULT6]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT78:%.*]] = shufflevector <8 x half> [[RESULT7]], <8 x half> [[RESULT8]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT1234:%.*]] = shufflevector <16 x half> [[RESULT12]], <16 x half> [[RESULT34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT5678:%.*]] = shufflevector <16 x half> [[RESULT56]], <16 x half> [[RESULT78]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT:%.*]] = shufflevector <32 x half> [[RESULT1234]], <32 x half> [[RESULT5678]], <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: ret <64 x half> [[RESULT]]

define <64 x half> @test_sins64(<64 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <64 x half> @__svml_sins64(<64 x half> %A)
  ret <64 x half> %0
}

; CHECK-LABEL: @test_sins16_mask
; CHECK: [[ARG1:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK1:%.*]] = shufflevector <16 x i16> %B, <16 x i16> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG1]], <8 x i16> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK2:%.*]] = shufflevector <16 x i16> %B, <16 x i16> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG2]], <8 x i16> [[MASK2]])
; CHECK: [[RESULT:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: ret <16 x half> [[RESULT]]

define <16 x half> @test_sins16_mask(<16 x half> %A, <16 x i16> %B) #0 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16_mask(<16 x half> %A, <16 x i16> %B)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sins32_mask
; CHECK: [[MASK:%.*]] = bitcast i32 %B to <32 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <32 x i1> [[MASK]], <32 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <32 x i16> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <32 x half> %C, <32 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK1:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG1]], <8 x i16> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x half> %C, <32 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK2:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG2]], <8 x i16> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x half> %C, <32 x half> undef,  <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[MASK3:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef,  <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG3]], <8 x i16> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x half> %C, <32 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK4:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG4]], <8 x i16> [[MASK4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT34:%.*]] = shufflevector <8 x half> [[RESULT3]], <8 x half> [[RESULT4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = shufflevector <16 x half> [[RESULT12]], <16 x half> [[RESULT34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT_COMBINED:%.*]] = select fast <32 x i1> [[MASK]], <32 x half> [[RESULT]], <32 x half> %A
; CHECK: ret <32 x half> [[RESULT_COMBINED]]

define <32 x half> @test_sins32_mask(<32 x half> %A, i32 zeroext %B, <32 x half> %C) #0 {
entry:
  %0 = bitcast i32 %B to <32 x i1>
  %1 = tail call fast svml_cc <32 x half> @__svml_sins32_mask(<32 x half> %A, <32 x i1> %0, <32 x half> %C)
  ret <32 x half> %1
}

; CHECK-LABEL: @test_sins64_mask
; CHECK: [[MASK:%.*]] = bitcast i64 %B to <64 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <64 x i1> [[MASK]], <64 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <64 x i16> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK1:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG1]], <8 x i16> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK2:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG2]], <8 x i16> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[MASK3:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG3]], <8 x i16> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK4:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG4]], <8 x i16> [[MASK4]])
; CHECK: [[ARG5:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39>
; CHECK: [[MASK5:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39>
; CHECK: [[RESULT5:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG5]], <8 x i16> [[MASK5]])
; CHECK: [[ARG6:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47>
; CHECK: [[MASK6:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47>
; CHECK: [[RESULT6:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG6]], <8 x i16> [[MASK6]])
; CHECK: [[ARG7:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55>
; CHECK: [[MASK7:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55>
; CHECK: [[RESULT7:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG7]], <8 x i16> [[MASK7]])
; CHECK: [[ARG8:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <8 x i32> <i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[MASK8:%.*]] = shufflevector <64 x i16> [[MASK_CAST]], <64 x i16> undef, <8 x i32> <i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT8:%.*]] = call fast svml_cc <8 x half> @__svml_sins8_mask(<8 x half> [[ARG8]], <8 x i16> [[MASK8]])
; CHECK: [[RESULT12:%.*]] = shufflevector <8 x half> [[RESULT1]], <8 x half> [[RESULT2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT34:%.*]] = shufflevector <8 x half> [[RESULT3]], <8 x half> [[RESULT4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT56:%.*]] = shufflevector <8 x half> [[RESULT5]], <8 x half> [[RESULT6]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT78:%.*]] = shufflevector <8 x half> [[RESULT7]], <8 x half> [[RESULT8]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT1234:%.*]] = shufflevector <16 x half> [[RESULT12]], <16 x half> [[RESULT34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT5678:%.*]] = shufflevector <16 x half> [[RESULT56]], <16 x half> [[RESULT78]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT:%.*]] = shufflevector <32 x half> [[RESULT1234]], <32 x half> [[RESULT5678]], <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT_COMBINED:%.*]] = select fast <64 x i1> [[MASK]], <64 x half> [[RESULT]], <64 x half> %A
; CHECK: ret <64 x half> [[RESULT_COMBINED]]

define <64 x half> @test_sins64_mask(<64 x half> %A, i64 zeroext %B, <64 x half> %C) #0 {
entry:
  %0 = bitcast i64 %B to <64 x i1>
  %1 = tail call fast svml_cc <64 x half> @__svml_sins64_mask(<64 x half> %A, <64 x i1> %0, <64 x half> %C)
  ret <64 x half> %1
}

; CHECK-LABEL: @test_sins64_mask_split_to_avx512
; CHECK: [[MASK:%.*]] = bitcast i64 %B to <64 x i1>
; CHECK: [[SRC1:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK1:%.*]] = shufflevector <64 x i1> [[MASK]], <64 x i1> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[ARG1:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT1:%.*]] = call fast svml_avx512_cc <32 x half> @__svml_sins32_mask(<32 x half> [[SRC1]], <32 x i1> [[MASK1]], <32 x half> [[ARG1]])
; CHECK: [[SRC2:%.*]] = shufflevector <64 x half> %A, <64 x half> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[MASK2:%.*]] = shufflevector <64 x i1> [[MASK]], <64 x i1> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[ARG2:%.*]] = shufflevector <64 x half> %C, <64 x half> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT2:%.*]] = call fast svml_avx512_cc <32 x half> @__svml_sins32_mask(<32 x half> [[SRC2]], <32 x i1> [[MASK2]], <32 x half> [[ARG2]])
; CHECK: [[RESULT:%.*]] = shufflevector <32 x half> [[RESULT1]], <32 x half> [[RESULT2]], <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: ret <64 x half> [[RESULT]]

define <64 x half> @test_sins64_mask_split_to_avx512(<64 x half> %A, i64 zeroext %B, <64 x half> %C) #1 {
entry:
  %0 = bitcast i64 %B to <64 x i1>
  %1 = tail call fast svml_cc <64 x half> @__svml_sins64_mask(<64 x half> %A, <64 x i1> %0, <64 x half> %C)
  ret <64 x half> %1
}

; CHECK-LABEL: @test_sincoss16
; CHECK: [[ARG1:%.*]] = shufflevector <16 x half> %a, <16 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x half> %a, <16 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG2]])
; CHECK: [[SIN1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 0
; CHECK: [[SIN2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 0
; CHECK: [[SIN:%.*]] = shufflevector <8 x half> [[SIN1]], <8 x half> [[SIN2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[TMP1:%.*]] = insertvalue { <16 x half>, <16 x half> } undef, <16 x half> [[SIN]], 0
; CHECK: [[COS1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 1
; CHECK: [[COS2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 1
; CHECK: [[COS:%.*]] = shufflevector <8 x half> [[COS1]], <8 x half> [[COS2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = insertvalue { <16 x half>, <16 x half> } [[TMP1]], <16 x half> [[COS]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 1
; CHECK: store <16 x half> [[COS_RET]], <16 x half>* %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 0
; CHECK: ret <16 x half> [[SIN_RET]]

define <16 x half> @test_sincoss16(<16 x half>* nocapture %p, <16 x half> %a) #0 {
entry:
  %0 = tail call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half> %a)
  %1 = extractvalue { <16 x half>, <16 x half> } %0, 1
  store <16 x half> %1, <16 x half>* %p, align 32
  %2 = extractvalue { <16 x half>, <16 x half> } %0, 0
  ret <16 x half> %2
}

; CHECK-LABEL: @test_sincoss32
; CHECK: [[ARG1:%.*]] = shufflevector <32 x half> %a, <32 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x half> %a, <32 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x half> %a, <32 x half> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x half> %a, <32 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha(<8 x half> [[ARG4]])
; CHECK: [[SIN1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 0
; CHECK: [[SIN2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 0
; CHECK: [[SIN3:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT3]], 0
; CHECK: [[SIN4:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT4]], 0
; CHECK: [[SIN12:%.*]] = shufflevector <8 x half> [[SIN1]], <8 x half> [[SIN2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[SIN34:%.*]] = shufflevector <8 x half> [[SIN3]], <8 x half> [[SIN4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[SIN:%.*]] = shufflevector <16 x half> [[SIN12]], <16 x half> [[SIN34]], <32 x i32>  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[TMP1:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> [[SIN]], 0
; CHECK: [[COS1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 1
; CHECK: [[COS2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 1
; CHECK: [[COS3:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT3]], 1
; CHECK: [[COS4:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT4]], 1
; CHECK: [[COS12:%.*]] = shufflevector <8 x half> [[COS1]], <8 x half> [[COS2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[COS34:%.*]] = shufflevector <8 x half> [[COS3]], <8 x half> [[COS4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[COS:%.*]] = shufflevector <16 x half> [[COS12]], <16 x half> [[COS34]], <32 x i32>  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT:%.*]] = insertvalue { <32 x half>, <32 x half> } [[TMP1]], <32 x half> [[COS]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
; CHECK: store <32 x half> [[COS_RET]], <32 x half>* %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
; CHECK: ret <32 x half> [[SIN_RET]]

define <32 x half> @test_sincoss32(<32 x half>* nocapture %p, <32 x half> %a) #0 {
entry:
  %0 = tail call svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32(<32 x half> %a)
  %1 = extractvalue { <32 x half>, <32 x half> } %0, 1
  store <32 x half> %1, <32 x half>* %p, align 32
  %2 = extractvalue { <32 x half>, <32 x half> } %0, 0
  ret <32 x half> %2
}

; CHECK-LABEL: @test_sincoss32_mask
; CHECK: [[MASK:%.*]] = bitcast i32 %D to <32 x i1>
; CHECK: [[TMP1:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> %B, 0
; CHECK: [[SRC:%.*]] = insertvalue { <32 x half>, <32 x half> } [[TMP1]], <32 x half> %C, 1
; CHECK: [[MASK_CAST:%.*]] = select <32 x i1> %0, <32 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <32 x i16> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <32 x half> %E, <32 x half> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK1:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha_mask(<8 x half> [[ARG1]], <8 x i16> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x half> %E, <32 x half> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK2:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT2:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha_mask(<8 x half> [[ARG2]], <8 x i16> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x half> %E, <32 x half> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[MASK3:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT3:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha_mask(<8 x half> [[ARG3]], <8 x i16> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x half> %E, <32 x half> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK4:%.*]] = shufflevector <32 x i16> [[MASK_CAST]], <32 x i16> undef, <8 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT4:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8_ha_mask(<8 x half> [[ARG4]], <8 x i16> [[MASK4]])
; CHECK: [[SIN1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 0
; CHECK: [[SIN2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 0
; CHECK: [[SIN3:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT3]], 0
; CHECK: [[SIN4:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT4]], 0
; CHECK: [[SIN_SRC:%.*]] = extractvalue { <32 x half>, <32 x half> } [[SRC]], 0
; CHECK: [[SIN12:%.*]] = shufflevector <8 x half> [[SIN1]], <8 x half> [[SIN2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[SIN34:%.*]] = shufflevector <8 x half> [[SIN3]], <8 x half> [[SIN4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[SIN:%.*]] = shufflevector <16 x half> [[SIN12]], <16 x half> [[SIN34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[SIN_COMBINED:%.*]] = select <32 x i1> [[MASK]], <32 x half> [[SIN]], <32 x half> [[SIN_SRC]]
; CHECK: [[TMP2:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> [[SIN_COMBINED]], 0
; CHECK: [[COS1:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT1]], 1
; CHECK: [[COS2:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT2]], 1
; CHECK: [[COS3:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT3]], 1
; CHECK: [[COS4:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT4]], 1
; CHECK: [[COS_SRC:%.*]] = extractvalue { <32 x half>, <32 x half> } [[SRC]], 1
; CHECK: [[COS12:%.*]] = shufflevector <8 x half> [[COS1]], <8 x half> [[COS2]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[COS34:%.*]] = shufflevector <8 x half> [[COS3]], <8 x half> [[COS4]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[COS:%.*]] = shufflevector <16 x half> [[COS12]], <16 x half> [[COS34]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[COS_COMBINED:%.*]] = select <32 x i1> [[MASK]], <32 x half> [[COS]], <32 x half> [[COS_SRC]]
; CHECK: [[RESULT:%.*]] = insertvalue { <32 x half>, <32 x half> } [[TMP2]], <32 x half> [[COS_COMBINED]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
; CHECK: store <32 x half> [[COS_RET]], <32 x half>* %A, align 64
; CHECK: [[SIN_RET:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
; CHECK: ret <32 x half> [[SIN_RET]]

define <32 x half> @test_sincoss32_mask(<32 x half>* nocapture %A, <32 x half> %B, <32 x half> %C, i32 zeroext %D, <32 x half> %E) #0 {
entry:
  %0 = bitcast i32 %D to <32 x i1>
  %1 = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> %B, 0
  %2 = insertvalue { <32 x half>, <32 x half> } %1, <32 x half> %C, 1
  %3 = tail call svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32_mask({ <32 x half>, <32 x half> } %2, <32 x i1> %0, <32 x half> %E)
  %4 = extractvalue { <32 x half>, <32 x half> } %3, 1
  store <32 x half> %4, <32 x half>* %A, align 64
  %5 = extractvalue { <32 x half>, <32 x half> } %3, 0
  ret <32 x half> %5
}

; CHECK-LABEL: @test_sincoss64_mask_split_to_avx512
; CHECK: [[MASK:%.*]] = bitcast i64 %D to <64 x i1>
; CHECK: [[TMP1:%.*]] = insertvalue { <64 x half>, <64 x half> } undef, <64 x half> %B, 0
; CHECK: [[SRC:%.*]] = insertvalue { <64 x half>, <64 x half> } [[TMP1]], <64 x half> %C, 1
; CHECK: [[SRC_SIN:%.*]] = extractvalue { <64 x half>, <64 x half> } [[SRC]], 0
; CHECK: [[SRC_COS:%.*]] = extractvalue { <64 x half>, <64 x half> } [[SRC]], 1
; CHECK: [[SRC1_SIN:%.*]] = shufflevector <64 x half> [[SRC_SIN]], <64 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[SRC1_TMP:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> [[SRC1_SIN]], 0
; CHECK: [[SRC1_COS:%.*]] = shufflevector <64 x half> [[SRC_COS]], <64 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[SRC1:%.*]] = insertvalue { <32 x half>, <32 x half> } [[SRC1_TMP]], <32 x half> [[SRC1_COS]], 1
; CHECK: [[MASK1:%.*]] = shufflevector <64 x i1> [[MASK]], <64 x i1> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[ARG1:%.*]] = shufflevector <64 x half> %E, <64 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT1:%.*]] = call svml_avx512_cc { <32 x half>, <32 x half> } @__svml_sincoss32_ha_mask({ <32 x half>, <32 x half> } [[SRC1]], <32 x i1> [[MASK1]], <32 x half> [[ARG1]])
; CHECK: [[SRC2_SIN:%.*]] = shufflevector <64 x half> [[SRC_SIN]], <64 x half> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[SRC2_TMP:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> [[SRC2_SIN]], 0
; CHECK: [[SRC2_COS:%.*]] = shufflevector <64 x half> [[SRC_COS]], <64 x half> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[SRC2:%.*]] = insertvalue { <32 x half>, <32 x half> } [[SRC2_TMP]], <32 x half> [[SRC2_COS]], 1
; CHECK: [[MASK2:%.*]] = shufflevector <64 x i1> [[MASK]], <64 x i1> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[ARG2:%.*]] = shufflevector <64 x half> %E, <64 x half> undef, <32 x i32> <i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT2:%.*]] = call svml_avx512_cc { <32 x half>, <32 x half> } @__svml_sincoss32_ha_mask({ <32 x half>, <32 x half> } [[SRC2]], <32 x i1> [[MASK2]], <32 x half> [[ARG2]])
; CHECK: [[SIN1:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT1]], 0
; CHECK: [[SIN2:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT2]], 0
; CHECK: [[SIN:%.*]] = shufflevector <32 x half> [[SIN1]], <32 x half> [[SIN2]], <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT_TMP:%.*]] = insertvalue { <64 x half>, <64 x half> } undef, <64 x half> [[SIN]], 0
; CHECK: [[COS1:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT1]], 1
; CHECK: [[COS2:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT2]], 1
; CHECK: [[COS:%.*]] = shufflevector <32 x half> [[COS1]], <32 x half> [[COS2]], <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62, i32 63>
; CHECK: [[RESULT:%.*]] = insertvalue { <64 x half>, <64 x half> } [[RESULT_TMP]], <64 x half> [[COS]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <64 x half>, <64 x half> } [[RESULT]], 1
; CHECK: store <64 x half> [[COS_RET]], <64 x half>* %A, align 64
; CHECK: [[SIN_RET:%.*]] = extractvalue { <64 x half>, <64 x half> } [[RESULT]], 0
; CHECK: ret <64 x half> [[SIN_RET]]

define <64 x half> @test_sincoss64_mask_split_to_avx512(<64 x half>* nocapture %A, <64 x half> %B, <64 x half> %C, i64 zeroext %D, <64 x half> %E) #1 {
entry:
  %0 = bitcast i64 %D to <64 x i1>
  %1 = insertvalue { <64 x half>, <64 x half> } undef, <64 x half> %B, 0
  %2 = insertvalue { <64 x half>, <64 x half> } %1, <64 x half> %C, 1
  %3 = tail call svml_cc { <64 x half>, <64 x half> } @__svml_sincoss64_mask({ <64 x half>, <64 x half> } %2, <64 x i1> %0, <64 x half> %E)
  %4 = extractvalue { <64 x half>, <64 x half> } %3, 1
  store <64 x half> %4, <64 x half>* %A, align 64
  %5 = extractvalue { <64 x half>, <64 x half> } %3, 0
  ret <64 x half> %5
}

declare svml_cc <16 x half> @__svml_sins16(<16 x half>)

declare svml_cc <32 x half> @__svml_sins32(<32 x half>)

declare svml_cc <64 x half> @__svml_sins64(<64 x half>)

declare svml_cc <16 x half> @__svml_sins16_mask(<16 x half>, <16 x i16>)

declare svml_cc <32 x half> @__svml_sins32_mask(<32 x half>, <32 x i1>, <32 x half>)

declare svml_cc <64 x half> @__svml_sins64_mask(<64 x half>, <64 x i1>, <64 x half>)

declare svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half>)

declare svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32(<32 x half>)

declare svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32_mask({ <32 x half>, <32 x half> }, <32 x i1>, <32 x half>)

declare svml_cc { <64 x half>, <64 x half> } @__svml_sincoss64_mask({ <64 x half>, <64 x half> }, <64 x i1>, <64 x half>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
