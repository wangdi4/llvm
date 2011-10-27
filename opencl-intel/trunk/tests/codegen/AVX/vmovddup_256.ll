; RUN: llc -mcpu=sandybridge < %s | FileCheck %s

define <2 x double> @test1(<2 x double>* nocapture %f) nounwind readonly {
; CHECK: test1
; CHECK: vmovddup
  %t0 = load <2 x double>* %f
  %t1 = extractelement <2 x double> %t0, i32 0
  %t2 = insertelement <2 x double> undef, double %t1, i32 0
  %t3 = insertelement <2 x double> %t2, double %t1, i32 1
  ret <2 x double> %t3
}

define <4 x double> @test2(<4 x double>* nocapture %f) nounwind readonly {
; CHECK: test2
; CHECK: vmovddup
  %t0 = load <4 x double>* %f
  %t1 = extractelement <4 x double> %t0, i32 0
  %t2 = extractelement <4 x double> %t0, i32 2
  %t3 = insertelement <4 x double> undef, double %t1, i32 0
  %t4 = insertelement <4 x double> %t3, double %t1, i32 1
  %t5 = insertelement <4 x double> %t4, double %t2, i32 2
  %t6 = insertelement <4 x double> %t5, double %t2, i32 3
  ret <4 x double> %t6
}

; mask with undefs
define <4 x double> @test3(<4 x double>* nocapture %f) nounwind readonly {
; CHECK: test3
; CHECK: vmovddup
  %t0 = load <4 x double>* %f
  %t1 = extractelement <4 x double> %t0, i32 0
  %t2 = extractelement <4 x double> %t0, i32 2
  %t3 = insertelement <4 x double> undef, double %t1, i32 0
  %t4 = insertelement <4 x double> %t3, double %t1, i32 1
  %t5 = insertelement <4 x double> %t4, double %t2, i32 2
  ret <4 x double> %t5
}

define <4 x i64> @test4(<4 x i64>* nocapture %f) nounwind readonly {
; CHECK: test4
; CHECK: vmovddup
  %t0 = load <4 x i64>* %f
  %t1 = extractelement <4 x i64> %t0, i32 0
  %t2 = extractelement <4 x i64> %t0, i32 2
  %t3 = insertelement <4 x i64> undef, i64 %t1, i32 0
  %t4 = insertelement <4 x i64> %t3, i64 %t1, i32 1
  %t5 = insertelement <4 x i64> %t4, i64 %t2, i32 2
  %t6 = insertelement <4 x i64> %t5, i64 %t2, i32 3
  ret <4 x i64> %t6
}

define <8 x float> @test5(<8 x float>* nocapture %f) nounwind readonly {
; CHECK: test5
; CHECK: vmovsldup
  %t0 = load <8 x float>* %f
  %t1 = extractelement <8 x float> %t0, i32 0
  %t2 = extractelement <8 x float> %t0, i32 2
  %t3 = extractelement <8 x float> %t0, i32 4
  %t4 = extractelement <8 x float> %t0, i32 6
  %t5 = insertelement <8 x float> undef, float %t1, i32 0
  %t6 = insertelement <8 x float> %t5, float %t1, i32 1
  %t7 = insertelement <8 x float> %t6, float %t2, i32 2
  %t8 = insertelement <8 x float> %t7, float %t2, i32 3
  %t9 = insertelement <8 x float> %t8, float %t3, i32 4
  %t10 = insertelement <8 x float> %t9, float %t3, i32 5
  %t11 = insertelement <8 x float> %t10, float %t4, i32 6
  %t12 = insertelement <8 x float> %t11, float %t4, i32 7
  ret <8 x float> %t12
}

define <8 x float> @test6(<8 x float> %t0) nounwind readonly {
; CHECK: test6
; CHECK: vmovsldup
  %t1 = extractelement <8 x float> %t0, i32 0
  %t2 = extractelement <8 x float> %t0, i32 2
  %t3 = extractelement <8 x float> %t0, i32 4
  %t4 = extractelement <8 x float> %t0, i32 6
  %t5 = insertelement <8 x float> undef, float %t1, i32 0
  %t6 = insertelement <8 x float> %t5, float %t1, i32 1
  %t7 = insertelement <8 x float> %t6, float %t2, i32 2
  %t8 = insertelement <8 x float> %t7, float %t2, i32 3
  %t9 = insertelement <8 x float> %t8, float %t3, i32 4
  %t10 = insertelement <8 x float> %t9, float %t3, i32 5
  %t11 = insertelement <8 x float> %t10, float %t4, i32 6
  %t12 = insertelement <8 x float> %t11, float %t4, i32 7
  ret <8 x float> %t12
}

define <8 x i32> @test7(<8 x i32> %t0) nounwind readonly {
; CHECK: test7
; CHECK: vmovsldup
  %t1 = extractelement <8 x i32> %t0, i32 0
  %t2 = extractelement <8 x i32> %t0, i32 2
  %t3 = extractelement <8 x i32> %t0, i32 4
  %t4 = extractelement <8 x i32> %t0, i32 6
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t8, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

define <8 x i32> @test8(<8 x i32>* nocapture %f) nounwind readonly {
; CHECK: test8
; CHECK: vmovsldup
  %t0 = load <8 x i32>* %f
  %t1 = extractelement <8 x i32> %t0, i32 0
  %t2 = extractelement <8 x i32> %t0, i32 2
  %t3 = extractelement <8 x i32> %t0, i32 4
  %t4 = extractelement <8 x i32> %t0, i32 6
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t8, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

; mask with undef
define <8 x i32> @test9(<8 x i32> %t0) nounwind readonly {
; CHECK: test9
; CHECK: vmovsldup
  %t1 = extractelement <8 x i32> %t0, i32 0
  %t2 = extractelement <8 x i32> %t0, i32 2
  %t3 = extractelement <8 x i32> %t0, i32 4
  %t4 = extractelement <8 x i32> %t0, i32 6
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  ; %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t7, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

; even when upper half is undef, use vmovsldup
define <8 x i32> @test10(<8 x i32> %t0) nounwind readonly {
; CHECK: test10
; CHECK: vmovsldup
; CHECK: ret
  %t1 = extractelement <8 x i32> %t0, i32 0
  %t2 = extractelement <8 x i32> %t0, i32 2
  %t3 = extractelement <8 x i32> %t0, i32 4
  %t4 = extractelement <8 x i32> %t0, i32 6
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  ; %t9 = insertelement <8 x i32> %t7, i32 %t3, i32 4
  ; %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  ; %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  ; %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t8
}

define <8 x i32> @test11(<8 x i32>* nocapture %f) nounwind readonly {
; CHECK: test11
; CHECK: vmovaps
  %t0 = load <8 x i32>* %f
  %t1 = extractelement <8 x i32> %t0, i32 0
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  ret <8 x i32> %t5
}

define <8 x float> @test12(<8 x float>* nocapture %f) nounwind readonly {
; CHECK: test12
; CHECK: vmovshdup
  %t0 = load <8 x float>* %f
  %t1 = extractelement <8 x float> %t0, i32 1
  %t2 = extractelement <8 x float> %t0, i32 3
  %t3 = extractelement <8 x float> %t0, i32 5
  %t4 = extractelement <8 x float> %t0, i32 7
  %t5 = insertelement <8 x float> undef, float %t1, i32 0
  %t6 = insertelement <8 x float> %t5, float %t1, i32 1
  %t7 = insertelement <8 x float> %t6, float %t2, i32 2
  %t8 = insertelement <8 x float> %t7, float %t2, i32 3
  %t9 = insertelement <8 x float> %t8, float %t3, i32 4
  %t10 = insertelement <8 x float> %t9, float %t3, i32 5
  %t11 = insertelement <8 x float> %t10, float %t4, i32 6
  %t12 = insertelement <8 x float> %t11, float %t4, i32 7
  ret <8 x float> %t12
}

define <8 x float> @test13(<8 x float> %t0) nounwind readonly {
; CHECK: test13
; CHECK: vmovshdup
  %t1 = extractelement <8 x float> %t0, i32 1
  %t2 = extractelement <8 x float> %t0, i32 3
  %t3 = extractelement <8 x float> %t0, i32 5
  %t4 = extractelement <8 x float> %t0, i32 7
  %t5 = insertelement <8 x float> undef, float %t1, i32 0
  %t6 = insertelement <8 x float> %t5, float %t1, i32 1
  %t7 = insertelement <8 x float> %t6, float %t2, i32 2
  %t8 = insertelement <8 x float> %t7, float %t2, i32 3
  %t9 = insertelement <8 x float> %t8, float %t3, i32 4
  %t10 = insertelement <8 x float> %t9, float %t3, i32 5
  %t11 = insertelement <8 x float> %t10, float %t4, i32 6
  %t12 = insertelement <8 x float> %t11, float %t4, i32 7
  ret <8 x float> %t12
}

define <8 x i32> @test14(<8 x i32> %t0) nounwind readonly {
; CHECK: test14
; CHECK: vmovshdup
  %t1 = extractelement <8 x i32> %t0, i32 1
  %t2 = extractelement <8 x i32> %t0, i32 3
  %t3 = extractelement <8 x i32> %t0, i32 5
  %t4 = extractelement <8 x i32> %t0, i32 7
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t8, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

define <8 x i32> @test15(<8 x i32>* nocapture %f) nounwind readonly {
; CHECK: test15
; CHECK: vmovshdup
  %t0 = load <8 x i32>* %f
  %t1 = extractelement <8 x i32> %t0, i32 1
  %t2 = extractelement <8 x i32> %t0, i32 3
  %t3 = extractelement <8 x i32> %t0, i32 5
  %t4 = extractelement <8 x i32> %t0, i32 7
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t8, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

; mask with undef
define <8 x i32> @test16(<8 x i32> %t0) nounwind readonly {
; CHECK: test16
; CHECK: vmovshdup
  %t1 = extractelement <8 x i32> %t0, i32 1
  %t2 = extractelement <8 x i32> %t0, i32 3
  %t3 = extractelement <8 x i32> %t0, i32 5
  %t4 = extractelement <8 x i32> %t0, i32 7
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  ; %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  %t9 = insertelement <8 x i32> %t7, i32 %t3, i32 4
  %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t12
}

; even when upper half is undef, use vmovshdup
define <8 x i32> @test17(<8 x i32> %t0) nounwind readonly {
; CHECK: test17
; CHECK: vmovshdup
  %t1 = extractelement <8 x i32> %t0, i32 1
  %t2 = extractelement <8 x i32> %t0, i32 3
  %t3 = extractelement <8 x i32> %t0, i32 5
  %t4 = extractelement <8 x i32> %t0, i32 7
  %t5 = insertelement <8 x i32> undef, i32 %t1, i32 0
  %t6 = insertelement <8 x i32> %t5, i32 %t1, i32 1
  %t7 = insertelement <8 x i32> %t6, i32 %t2, i32 2
  %t8 = insertelement <8 x i32> %t7, i32 %t2, i32 3
  ; %t9 = insertelement <8 x i32> %t7, i32 %t3, i32 4
  ; %t10 = insertelement <8 x i32> %t9, i32 %t3, i32 5
  ; %t11 = insertelement <8 x i32> %t10, i32 %t4, i32 6
  ; %t12 = insertelement <8 x i32> %t11, i32 %t4, i32 7
  ret <8 x i32> %t8
}
