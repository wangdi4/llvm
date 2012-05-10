; XFAIL: *
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

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

define <8 x i64> @B_i64(<8 x i64> %a) nounwind ssp {
entry:
; KNC: valignd $8, %zmm0, %zmm0, %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 1, i32 undef, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @B_f64(<8 x double> %a) nounwind ssp {
entry:
; KNC: valignd $8, %zmm0, %zmm0, %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> undef, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 1, i32 undef, i32 undef>
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
