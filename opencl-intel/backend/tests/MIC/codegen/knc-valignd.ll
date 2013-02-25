; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

define <16 x i32> @test2(<16 x i32> %A, <16 x i32> %B) nounwind {
; KNC: test2:
; KNC: valignd $1, %zmm0, %zmm1, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16 >
  ret <16 x i32> %C
}

define <16 x i32> @test3(<16 x i32> %A, <16 x i32> %B) nounwind {
; KNC: test3:
; KNC: valignd $1, %zmm0, %zmm1, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16 >
	ret <16 x i32> %C
}

define <16 x i32> @test4(<16 x i32> %A, <16 x i32> %B) nounwind {
; KNC: test4:
; KNC: valignd $14, %zmm1, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> %B, <16 x i32> < i32 30, i32 31,  i32 undef, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13>
	ret <16 x i32> %C
}

define <16 x float> @test5(<16 x float> %A, <16 x float> %B) nounwind {
; KNC: test5:
; KNC: valignd $14, %zmm1, %zmm0, %zmm0
  %C = shufflevector <16 x float> %A, <16 x float> %B, <16 x i32> < i32 30, i32 31,  i32 undef, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13>
	ret <16 x float> %C
}

define <16 x i32> @test6(<16 x i32> %A) nounwind {
; KNC: test6:
; KNC: valignd $1, %zmm0, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> undef, <16 x i32> < i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0 >
	ret <16 x i32> %C
}

define <16 x i32> @test7(<16 x i32> %A) nounwind {
; KNC: test7:
; KNC: valignd $15, %zmm0, %zmm0, %zmm0
  %C = shufflevector <16 x i32> %A, <16 x i32> undef, <16 x i32> < i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14 >
	ret <16 x i32> %C
}

define <16 x i32> @A_i32(<16 x i32> %a) nounwind ssp {
entry:
; KNC: valignd $15, %zmm0, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 15, i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @A_f32(<16 x float> %a) nounwind ssp {
entry:
; KNC: valignd $15, %zmm0, %zmm0, %zmm0
  %m = shufflevector <16 x float> %a, <16 x float> undef, <16 x i32> <i32 15, i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <16 x i32> @B_i32(<16 x i32> %a) nounwind ssp {
entry:
; KNC: valignd $7, %zmm0, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 11, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 3, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @B_f32(<16 x float> %a) nounwind ssp {
entry:
; KNC: valignd $7, %zmm0, %zmm0, %zmm0
  %m = shufflevector <16 x float> %a, <16 x float> undef, <16 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 11, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 3, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <16 x i32> @C_i32(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNC: valignd $15, %zmm0, %zmm1, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> %b, <16 x i32> <i32 15, i32 16, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @C_f32(<16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNC: valignd $15, %zmm0, %zmm1, %zmm0
  %m = shufflevector <16 x float> %a, <16 x float> %b, <16 x i32> <i32 15, i32 16, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <16 x i32> @D_i32(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNC: valignd $7, %zmm0, %zmm1, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 11, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 19, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @D_f32(<16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNC: valignd $7, %zmm0, %zmm1, %zmm0
  %m = shufflevector <16 x float> %a, <16 x float> %b, <16 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 11, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 19, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <8 x i64> @A_i64(<8 x i64> %a) nounwind ssp {
entry:
; KNC: valignd $14, %zmm0, %zmm0, %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> <i32 7, i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @A_f64(<8 x double> %a) nounwind ssp {
entry:
; KNC: valignd $14, %zmm0, %zmm0, %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> undef, <8 x i32> <i32 7, i32 0, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x double> %m
}

define <8 x i64> @C_i64(<8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: valignd $14, %zmm0, %zmm1, %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> %b, <8 x i32> <i32 7, i32 8, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @C_f64(<8 x double> %a, <8 x double> %b)  nounwind ssp {
entry:
; KNC: valignd $14, %zmm0, %zmm1, %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> %b, <8 x i32> <i32 7, i32 8, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x double> %m
}

define <8 x i64> @D_i64(<8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: valignd $8, %zmm0, %zmm1, %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> %b, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 9, i32 undef, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @D_f64(<8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNC: valignd $8, %zmm0, %zmm1, %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> %b, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 9, i32 undef, i32 undef>
  ret <8 x double> %m
}
