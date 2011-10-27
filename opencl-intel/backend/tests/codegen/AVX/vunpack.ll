; RUN: llc -mcpu=sandybridge < %s | FileCheck %s
target triple="i686-pc-win32"

; v8f32

define <8 x float> @test_f32_1(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_1
; CHECK: vunpcklps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 0, i32 8, i32 1, i32 9, i32 4, i32 12, i32 5, i32 13>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_2(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_2
; CHECK: vunpcklps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 12, i32 5, i32 13>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_3(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_3
; CHECK: vunpckhps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 2, i32 10, i32 3, i32 11, i32 6, i32 14, i32 7, i32 15>
  ret <8 x float> %t1
}

; v8i32

define <8 x i32> @test_i32_1(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_1
; CHECK: vunpcklps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 0, i32 8, i32 1, i32 9, i32 4, i32 12, i32 5, i32 13>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_2(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_2
; CHECK: vunpcklps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 12, i32 5, i32 13>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_3(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_3
; CHECK: vunpckhps %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 2, i32 10, i32 3, i32 11, i32 6, i32 14, i32 7, i32 15>
  ret <8 x i32> %t1
}

; v4f64

define <4 x double> @test_f64_1(<4 x double> %v1, <4 x double> %v2) nounwind readonly {
; CHECK: test_f64_1
; CHECK: vunpcklpd %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <4 x double> %v1, <4 x double> %v2, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  ret <4 x double> %t1
}

define <4 x double> @test_f64_2(<4 x double> %v1, <4 x double> %v2) nounwind readonly {
; CHECK: test_f64_2
; CHECK: vunpckhpd %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <4 x double> %v1, <4 x double> %v2, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  ret <4 x double> %t1
}

; v4i64

define <4 x i64> @test_i64_1(<4 x i64> %v1, <4 x i64> %v2) nounwind readonly {
; CHECK: test_i64_1
; CHECK: vunpcklpd %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <4 x i64> %v1, <4 x i64> %v2, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  ret <4 x i64> %t1
}

define <4 x i64> @test_i64_2(<4 x i64> %v1, <4 x i64> %v2) nounwind readonly {
; CHECK: test_i64_2
; CHECK: vunpckhpd %ymm1, %ymm0, %ymm0
  %t1 = shufflevector <4 x i64> %v1, <4 x i64> %v2, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  ret <4 x i64> %t1
}
