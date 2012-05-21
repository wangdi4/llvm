; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC


define <16 x i32> @A(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
;
; KNC: vpermf32x4 $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @D(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
;
; KNC: vpermf32x4 $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 undef, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @F(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $196, $14, %v0, %v0
;
; KNC: vpermf32x4 $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 9, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @E(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
;
; KNC: vpermf32x4 $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @B(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $225, $14, %v0, %v0
;
; KNC:_const_0:
; KNC: .long 9
; KNC: .long 8
; KNC: .long 10
; KNC: .long 11
; KNC: .long 13
; KNC: .long 12
; KNC: .long 14
; KNC: .long 15
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: .long 0
; KNC: vmovaps _const_{{[0-9]}}(%rip), [[R0:%zmm[0-9]+]]
; KNC: vpermd %zmm0, [[R0]], %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 9, i32 8, i32 10, i32 11, i32 13, i32 12, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @C(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
;
; KNC: vpermf32x4 $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> undef, <16 x i32> %a, <16 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <8 x i64> @G(<8 x i64> %a) nounwind ssp {
entry:
; KNC: vpermf32x4 $177, %zmm0, %zmm0 
  %m = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 0, i32 1, i32 6, i32 7, i32 4, i32 5>
  ret <8 x i64> %m
}

define <8 x double> @B_f64(<8 x double> %a) nounwind ssp {
entry:
; KNC: vpermf32x4 $2, %zmm0, %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> undef, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 1, i32 undef, i32 undef>
  ret <8 x double> %m
}

define <8 x i64> @B_i64(<8 x i64> %a) nounwind ssp {
entry:
; KNC: vpermf32x4 $2, %zmm0, %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> <i32 undef, i32 5, i32 undef, i32 undef, i32 undef, i32 1, i32 undef, i32 undef>
  ret <8 x i64> %m
}


