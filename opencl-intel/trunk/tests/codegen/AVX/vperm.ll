; RUN: llc -mcpu=sandybridge < %s | FileCheck %s

; v4f64

define <4 x double> @test_f64_1(<4 x double> %v1, <4 x double> %v2) nounwind readonly {
; CHECK: test_f64_1
; CHECK: vperm2f128 $33
  %t1 = shufflevector <4 x double> %v1, <4 x double> %v2, <4 x i32> <i32 2, i32 3, i32 4, i32 5>
  ret <4 x double> %t1
}

; v8f32

define <8 x float> @test_f32_1(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_1
; CHECK: vperm2f128 $0, %ymm0, %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_2(<8 x float>* nocapture %v1, <8 x float>* nocapture %v2) nounwind readonly {
; CHECK: test_f32_2
; CHECK; vmovaps
  %t1 = load <8 x float>* %v1
  %t2 = load <8 x float>* %v2
  %t3 = shufflevector <8 x float> %t1, <8 x float> %t2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x float> %t3
}

define <8 x float> @test_f32_3(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_3
; CHECK: vperm2f128 $32
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_4(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_4
; CHECK: vperm2f128 $48
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 12, i32 13, i32 14, i32 15>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_5(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_5
; CHECK: vperm2f128 $1
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_6(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_6
; CHECK: vperm2f128 $17
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_7(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_7
; CHECK: vperm2f128 $33
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_8(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_8
; CHECK: vperm2f128 $49
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_9(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_9
; CHECK: vperm2f128 $2
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 0, i32 1, i32 2, i32 3>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_10(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_10
; CHECK: vperm2f128 $18
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 4, i32 5, i32 6, i32 7>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_11(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_11
; CHECK: vperm2f128 $0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 8, i32 9, i32 10, i32 11>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_12(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_12
; CHECK: vmovaps
; CHECK-NOT: vperm2f128
; CHECK: ret
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_13(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_13
; CHECK: vperm2f128 $3
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_14(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_14
; CHECK: vperm2f128 $19
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 4, i32 5, i32 6, i32 7>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_15(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_15
; CHECK: vperm2f128 $1
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11>
  ret <8 x float> %t1
}

define <8 x float> @test_f32_16(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_f32_16
; CHECK: vperm2f128 $17
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 12, i32 13, i32 14, i32 15>
  ret <8 x float> %t1
}

; v8i32

define <8 x i32> @test_i32_1(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_1
; CHECK: vperm2f128 $0, %ymm0, %ymm0, %ymm0
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_2(<8 x i32>* nocapture %v1, <8 x i32>* nocapture %v2) nounwind readonly {
; CHECK: test_i32_2
; CHECK; vmovaps
  %t1 = load <8 x i32>* %v1
  %t2 = load <8 x i32>* %v2
  %t3 = shufflevector <8 x i32> %t1, <8 x i32> %t2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %t3
}

define <8 x i32> @test_i32_3(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_3
; CHECK: vperm2f128 $32
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_4(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_4
; CHECK: vperm2f128 $48
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_5(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_5
; CHECK: vperm2f128 $1
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_6(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_6
; CHECK: vperm2f128 $17
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_7(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_7
; CHECK: vperm2f128 $33
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_8(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_8
; CHECK: vperm2f128 $49
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_9(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_9
; CHECK: vperm2f128 $2
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_10(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_10
; CHECK: vperm2f128 $18
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_11(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_11
; CHECK: vperm2f128 $0
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 8, i32 9, i32 10, i32 11>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_12(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_12
; CHECK: vmovaps
; CHECK-NOT: vperm2f128
; CHECK: ret
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_13(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_13
; CHECK: vperm2f128 $3
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_14(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_14
; CHECK: vperm2f128 $19
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_15(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_15
; CHECK: vperm2f128 $1
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11>
  ret <8 x i32> %t1
}

define <8 x i32> @test_i32_16(<8 x i32> %v1, <8 x i32> %v2) nounwind readonly {
; CHECK: test_i32_16
; CHECK: vperm2f128 $17
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> %v2, <8 x i32> <i32 12, i32 13, i32 14, i32 15, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i32> %t1
}

; Some case where mask contains undefs

define <8 x float> @test_undef_1(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_undef_1
; CHECK: vperm2f128 $48
  %t3 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 12, i32 13, i32 14, i32 15>
  ret <8 x float> %t3
}

define <8 x float> @test_undef_2(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_undef_2
; CHECK: vperm2f128 $48
  %t3 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 undef, i32 undef, i32 2, i32 3, i32 undef, i32 undef, i32 14, i32 15>
  ret <8 x float> %t3
}
