; Check split SVML calls are generated correctly when needed
; RUN: opt -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sinf8
; CHECK: [[ARG1:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG2]])
; CHECK: [[RESULT:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_sinf8(<8 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_sinf16
; CHECK: [[ARG1:%.*]] = shufflevector <16 x float> %A, <16 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x float> %A, <16 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG2]])
; CHECK: [[ARG3:%.*]] = shufflevector <16 x float> %A, <16 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG3]])
; CHECK: [[ARG4:%.*]] = shufflevector <16 x float> %A, <16 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: ret <16 x float> [[RESULT]]

define <16 x float> @test_sinf16(<16 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <16 x float> @__svml_sinf16(<16 x float> %A)
  ret <16 x float> %0
}

; CHECK-LABEL: @test_sinf32
; CHECK: [[ARG1:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG4]])
; CHECK: [[ARG5:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[RESULT5:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG5]])
; CHECK: [[ARG6:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT6:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG6]])
; CHECK: [[ARG7:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[RESULT7:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG7]])
; CHECK: [[ARG8:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT8:%.*]] = call svml_cc <4 x float> @__svml_sinf4(<4 x float> [[ARG8]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT56:%.*]] = shufflevector <4 x float> [[RESULT5]], <4 x float> [[RESULT6]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT78:%.*]] = shufflevector <4 x float> [[RESULT7]], <4 x float> [[RESULT8]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1234:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT5678:%.*]] = shufflevector <8 x float> [[RESULT56]], <8 x float> [[RESULT78]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = shufflevector <16 x float> [[RESULT1234]], <16 x float> [[RESULT5678]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: ret <32 x float> [[RESULT]]

define <32 x float> @test_sinf32(<32 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <32 x float> @__svml_sinf32(<32 x float> %A)
  ret <32 x float> %0
}

; CHECK-LABEL: @test_sinf8_mask
; CHECK: [[ARG1:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <8 x i32> %B, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG1]], <4 x i32> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <8 x i32> %B, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG2]], <4 x i32> [[MASK2]])
; CHECK: [[RESULT:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_sinf8_mask(<8 x float> %A, <8 x i32> %B) #0 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8_mask(<8 x float> %A, <8 x i32> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_sinf16_mask
; CHECK: [[MASK:%.*]] = bitcast i16 %B to <16 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <16 x i1> [[MASK]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>, <16 x i32> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG1]], <4 x i32> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG2]], <4 x i32> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[MASK3:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG3]], <4 x i32> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK4:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG4]], <4 x i32> [[MASK4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT_COMBINED:%.*]] = select <16 x i1> [[MASK]], <16 x float> [[RESULT]], <16 x float> %A
; CHECK: ret <16 x float> [[RESULT_COMBINED]]

define <16 x float> @test_sinf16_mask(<16 x float> %A, i16 zeroext %B, <16 x float> %C) #0 {
entry:
  %0 = bitcast i16 %B to <16 x i1>
  %1 = tail call fast svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> %A, <16 x i1> %0, <16 x float> %C)
  ret <16 x float> %1
}

; CHECK-LABEL: @test_sinf32_mask
; CHECK: [[MASK:%.*]] = bitcast i32 %B to <32 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <32 x i1> [[MASK]], <32 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>, <32 x i32> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG1]], <4 x i32> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG2]], <4 x i32> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[MASK3:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG3]], <4 x i32> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK4:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG4]], <4 x i32> [[MASK4]])
; CHECK: [[ARG5:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[MASK5:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[RESULT5:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG5]], <4 x i32> [[MASK5]])
; CHECK: [[ARG6:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[MASK6:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[RESULT6:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG6]], <4 x i32> [[MASK6]])
; CHECK: [[ARG7:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[MASK7:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[RESULT7:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG7]], <4 x i32> [[MASK7]])
; CHECK: [[ARG8:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK8:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT8:%.*]] = call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> [[ARG8]], <4 x i32> [[MASK8]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT56:%.*]] = shufflevector <4 x float> [[RESULT5]], <4 x float> [[RESULT6]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT78:%.*]] = shufflevector <4 x float> [[RESULT7]], <4 x float> [[RESULT8]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT1234:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT5678:%.*]] = shufflevector <8 x float> [[RESULT56]], <8 x float> [[RESULT78]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = shufflevector <16 x float> [[RESULT1234]], <16 x float> [[RESULT5678]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT_COMBINED:%.*]] = select <32 x i1> [[MASK]], <32 x float> [[RESULT]], <32 x float> %A
; CHECK: ret <32 x float> [[RESULT_COMBINED]]

define <32 x float> @test_sinf32_mask(<32 x float> %A, i32 zeroext %B, <32 x float> %C) #0 {
entry:
  %0 = bitcast i32 %B to <32 x i1>
  %1 = tail call fast svml_cc <32 x float> @__svml_sinf32_mask(<32 x float> %A, <32 x i1> %0, <32 x float> %C)
  ret <32 x float> %1
}

; CHECK-LABEL: @test_sinf32_mask_split_to_avx512
; CHECK: [[MASK:%.*]] = bitcast i32 %B to <32 x i1>
; CHECK: [[SRC1:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK1:%.*]] = shufflevector <32 x i1> [[MASK]], <32 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[ARG1:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT1:%.*]] = call svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> [[SRC1]], <16 x i1> [[MASK1]], <16 x float> [[ARG1]])
; CHECK: [[SRC2:%.*]] = shufflevector <32 x float> %A, <32 x float> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK2:%.*]] = shufflevector <32 x i1> [[MASK]], <32 x i1> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[ARG2:%.*]] = shufflevector <32 x float> %C, <32 x float> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[RESULT2:%.*]] = call svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> [[SRC2]], <16 x i1> [[MASK2]], <16 x float> [[ARG2]])
; CHECK: [[RESULT:%.*]] = shufflevector <16 x float> [[RESULT1]], <16 x float> [[RESULT2]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: ret <32 x float> [[RESULT]]

define <32 x float> @test_sinf32_mask_split_to_avx512(<32 x float> %A, i32 zeroext %B, <32 x float> %C) #1 {
entry:
  %0 = bitcast i32 %B to <32 x i1>
  %1 = tail call fast svml_cc <32 x float> @__svml_sinf32_mask(<32 x float> %A, <32 x i1> %0, <32 x float> %C)
  ret <32 x float> %1
}

; CHECK-LABEL: @test_idiv8
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %a to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %b to <8 x i32>
; CHECK: [[DIVIDEND1:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[DIVISOR1:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT1:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND1]], <4 x i32> [[DIVISOR1]])
; CHECK: [[DIVIDEND2:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR2:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT2:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND2]], <4 x i32> [[DIVISOR2]])
; CHECK: [[QUOTIENT:%.*]] = shufflevector <4 x i32> [[QUOTIENT1]], <4 x i32> [[QUOTIENT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
; CHECK: ret <4 x i64> [[QUOTIENT_CAST]]

define <4 x i64> @test_idiv8(<4 x i64> %a, <4 x i64> %b) #0 {
entry:
  %0 = bitcast <4 x i64> %a to <8 x i32>
  %1 = bitcast <4 x i64> %b to <8 x i32>
  %2 = tail call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> %0, <8 x i32> %1)
  %3 = bitcast <8 x i32> %2 to <4 x i64>
  ret <4 x i64> %3
}

; CHECK-LABEL: @test_idiv16
; CHECK: [[DIVIDEND:%.*]] = bitcast <8 x i64> %a to <16 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <8 x i64> %b to <16 x i32>
; CHECK: [[DIVIDEND1:%.*]] = shufflevector <16 x i32> [[DIVIDEND]], <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[DIVISOR1:%.*]] = shufflevector <16 x i32> [[DIVISOR]], <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT1:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND1]], <4 x i32> [[DIVISOR1]])
; CHECK: [[DIVIDEND2:%.*]] = shufflevector <16 x i32> [[DIVIDEND]], <16 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR2:%.*]] = shufflevector <16 x i32> [[DIVISOR]], <16 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT2:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND2]], <4 x i32> [[DIVISOR2]])
; CHECK: [[DIVIDEND3:%.*]] = shufflevector <16 x i32> [[DIVIDEND]], <16 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[DIVISOR3:%.*]] = shufflevector <16 x i32> [[DIVISOR]], <16 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[QUOTIENT3:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND3]], <4 x i32> [[DIVISOR3]])
; CHECK: [[DIVIDEND4:%.*]] = shufflevector <16 x i32> [[DIVIDEND]], <16 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[DIVISOR4:%.*]] = shufflevector <16 x i32> [[DIVISOR]], <16 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[QUOTIENT4:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND4]], <4 x i32> [[DIVISOR4]])
; CHECK: [[QUOTIENT12:%.*]] = shufflevector <4 x i32> [[QUOTIENT1]], <4 x i32> [[QUOTIENT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT34:%.*]] = shufflevector <4 x i32> [[QUOTIENT3]], <4 x i32> [[QUOTIENT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT:%.*]] = shufflevector <8 x i32> [[QUOTIENT12]], <8 x i32> [[QUOTIENT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <16 x i32> [[QUOTIENT]] to <8 x i64>
; CHECK: ret <8 x i64> [[QUOTIENT_CAST]]

define <8 x i64> @test_idiv16(<8 x i64> %a, <8 x i64> %b) #0 {
entry:
  %0 = bitcast <8 x i64> %a to <16 x i32>
  %1 = bitcast <8 x i64> %b to <16 x i32>
  %2 = tail call svml_cc <16 x i32> @__svml_idiv16(<16 x i32> %0, <16 x i32> %1)
  %3 = bitcast <16 x i32> %2 to <8 x i64>
  ret <8 x i64> %3
}

; CHECK-LABEL: @test_idiv8_mask
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %a to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %b to <8 x i32>
; CHECK: [[MASK:%.*]] = bitcast <4 x i64> %c to <8 x i32>
; CHECK: [[DIVIDEND1:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[DIVISOR1:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <8 x i32> [[MASK]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT1:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND1]], <4 x i32> [[DIVISOR1]], <4 x i32> [[MASK1]])
; CHECK: [[DIVIDEND2:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR2:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <8 x i32> [[MASK]], <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT2:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND2]], <4 x i32> [[DIVISOR2]], <4 x i32> [[MASK2]])
; CHECK: [[QUOTIENT:%.*]] = shufflevector <4 x i32> [[QUOTIENT1]], <4 x i32> [[QUOTIENT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
; CHECK: ret <4 x i64> [[QUOTIENT_CAST]]

define <4 x i64> @test_idiv8_mask(<4 x i64> %a, <4 x i64> %b, <4 x i64> %c) #0 {
entry:
  %0 = bitcast <4 x i64> %a to <8 x i32>
  %1 = bitcast <4 x i64> %b to <8 x i32>
  %2 = bitcast <4 x i64> %c to <8 x i32>
  %3 = tail call svml_cc <8 x i32> @__svml_idiv8_mask(<8 x i32> %0, <8 x i32> %1, <8 x i32> %2)
  %4 = bitcast <8 x i32> %3 to <4 x i64>
  ret <4 x i64> %4
}

; CHECK-LABEL: @test_idiv32_mask
; CHECK: [[SOURCE:%.*]] = bitcast <16 x i64> %a to <32 x i32>
; CHECK: [[MASK:%.*]] = bitcast i32 %b to <32 x i1>
; CHECK: [[DIVIDEND:%.*]] = bitcast <16 x i64> %c to <32 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <16 x i64> %d to <32 x i32>
; CHECK: [[MASK_CAST:%.*]] = select <32 x i1> [[MASK]], <32 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>, <32 x i32> zeroinitializer
; CHECK: [[DIVIDEND1:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[DIVISOR1:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT1:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND1]], <4 x i32> [[DIVISOR1]], <4 x i32> [[MASK1]])
; CHECK: [[DIVIDEND2:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR2:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT2:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND2]], <4 x i32> [[DIVISOR2]], <4 x i32> [[MASK2]])
; CHECK: [[DIVIDEND3:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[DIVISOR3:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[MASK3:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[QUOTIENT3:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND3]], <4 x i32> [[DIVISOR3]], <4 x i32> [[MASK3]])
; CHECK: [[DIVIDEND4:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[DIVISOR4:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK4:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[QUOTIENT4:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND4]], <4 x i32> [[DIVISOR4]], <4 x i32> [[MASK4]])
; CHECK: [[DIVIDEND5:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[DIVISOR5:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[MASK5:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 16, i32 17, i32 18, i32 19>
; CHECK: [[QUOTIENT5:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND5]], <4 x i32> [[DIVISOR5]], <4 x i32> [[MASK5]])
; CHECK: [[DIVIDEND6:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[DIVISOR6:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[MASK6:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 20, i32 21, i32 22, i32 23>
; CHECK: [[QUOTIENT6:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND6]], <4 x i32> [[DIVISOR6]], <4 x i32> [[MASK6]])
; CHECK: [[DIVIDEND7:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[DIVISOR7:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[MASK7:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 24, i32 25, i32 26, i32 27>
; CHECK: [[QUOTIENT7:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND7]], <4 x i32> [[DIVISOR7]], <4 x i32> [[MASK7]])
; CHECK: [[DIVIDEND8:%.*]] = shufflevector <32 x i32> [[DIVIDEND]], <32 x i32> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[DIVISOR8:%.*]] = shufflevector <32 x i32> [[DIVISOR]], <32 x i32> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[MASK8:%.*]] = shufflevector <32 x i32> [[MASK_CAST]], <32 x i32> undef, <4 x i32> <i32 28, i32 29, i32 30, i32 31>
; CHECK: [[QUOTIENT8:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_mask(<4 x i32> [[DIVIDEND8]], <4 x i32> [[DIVISOR8]], <4 x i32> [[MASK8]])
; CHECK: [[QUOTIENT12:%.*]] = shufflevector <4 x i32> [[QUOTIENT1]], <4 x i32> [[QUOTIENT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT34:%.*]] = shufflevector <4 x i32> [[QUOTIENT3]], <4 x i32> [[QUOTIENT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT56:%.*]] = shufflevector <4 x i32> [[QUOTIENT5]], <4 x i32> [[QUOTIENT6]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT78:%.*]] = shufflevector <4 x i32> [[QUOTIENT7]], <4 x i32> [[QUOTIENT8]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT1234:%.*]] = shufflevector <8 x i32> [[QUOTIENT12]], <8 x i32> [[QUOTIENT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[QUOTIENT5678:%.*]] = shufflevector <8 x i32> [[QUOTIENT56]], <8 x i32> [[QUOTIENT78]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[QUOTIENT:%.*]] = shufflevector <16 x i32> [[QUOTIENT1234]], <16 x i32> [[QUOTIENT5678]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK: [[QUOTIENT_COMBINED:%.*]] = select <32 x i1> [[MASK]], <32 x i32> [[QUOTIENT]], <32 x i32> [[SOURCE]]
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <32 x i32> [[QUOTIENT_COMBINED]] to <16 x i64>
; CHECK: ret <16 x i64> [[QUOTIENT_CAST]]

define <16 x i64> @test_idiv32_mask(<16 x i64> %a, i32 zeroext %b, <16 x i64> %c, <16 x i64> %d) #0 {
entry:
  %0 = bitcast <16 x i64> %a to <32 x i32>
  %1 = bitcast i32 %b to <32 x i1>
  %2 = bitcast <16 x i64> %c to <32 x i32>
  %3 = bitcast <16 x i64> %d to <32 x i32>
  %4 = tail call svml_cc <32 x i32> @__svml_idiv32_mask(<32 x i32> %0, <32 x i1> %1, <32 x i32> %2, <32 x i32> %3)
  %5 = bitcast <32 x i32> %4 to <16 x i64>
  ret <16 x i64> %5
}

declare svml_cc <8 x float> @__svml_sinf8(<8 x float>)

declare svml_cc <16 x float> @__svml_sinf16(<16 x float>)

declare svml_cc <32 x float> @__svml_sinf32(<32 x float>)

declare svml_cc <8 x float> @__svml_sinf8_mask(<8 x float>, <8 x i32>)

declare svml_cc <16 x float> @__svml_sinf16_mask(<16 x float>, <16 x i1>, <16 x float>)

declare svml_cc <32 x float> @__svml_sinf32_mask(<32 x float>, <32 x i1>, <32 x float>)

declare svml_cc <8 x i32> @__svml_idiv8(<8 x i32>, <8 x i32>)

declare svml_cc <16 x i32> @__svml_idiv16(<16 x i32>, <16 x i32>)

declare svml_cc <8 x i32> @__svml_idiv8_mask(<8 x i32>, <8 x i32>, <8 x i32>)

declare svml_cc <32 x i32> @__svml_idiv32_mask(<32 x i32>, <32 x i1>, <32 x i32>, <32 x i32>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
