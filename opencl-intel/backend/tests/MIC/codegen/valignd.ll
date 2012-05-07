; RUN: llc < %s -march=y86-64 -mcpu=knc | FileCheck %s

define <16 x i32> @test2(<16 x i32> %A, <16 x i32> %B) nounwind {
; CHECK: test2:
; CHECK: valignd $1, %zmm1, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16 >
  ret <16 x i32> %C
}

define <16 x i32> @test3(<16 x i32> %A, <16 x i32> %B) nounwind {
; CHECK: test3:
; CHECK: valignd $1, %zmm1, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16 >
	ret <16 x i32> %C
}

define <16 x i32> @test4(<16 x i32> %A, <16 x i32> %B) nounwind {
; CHECK: test4:
; CHECK: valignd $2, %zmm0, %zmm1, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 30, i32 31,  i32 undef, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13>
	ret <16 x i32> %C
}

define <16 x float> @test5(<16 x float> %A, <16 x float> %B) nounwind {
; CHECK: test5:
; CHECK: valignd $2, %zmm0, %zmm1, %zmm0
  %C = shufflevector <16 x float> %A, <16 x float> %B, <16 x i32> < i32 30, i32 31,  i32 undef, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13>
	ret <16 x float> %C
}

define <16 x i32> @test6(<16 x i32> %A) nounwind {
; CHECK: test6:
; CHECK: valignd $1, %zmm0, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> undef, <16 x i32> < i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0 >
	ret <16 x i32> %C
}

define <16 x i32> @test7(<16 x i32> %A) nounwind {
; CHECK: test7:
; CHECK: valignd $15, %zmm0, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> undef, <16 x i32> < i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14 >
	ret <16 x i32> %C
}
