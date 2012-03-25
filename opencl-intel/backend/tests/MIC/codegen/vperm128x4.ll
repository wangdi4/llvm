; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

define <16 x i32> @A(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
; KNC: vshuf128x32 $228, $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @B(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $225, $14, %v0, %v0
; KNC: vshuf128x32 $225, $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 9, i32 8, i32 10, i32 11, i32 13, i32 12, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @C(<16 x i32> %a) nounwind ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, %v0
; KNC: vshuf128x32 $228, $14, %zmm0, %zmm0
  %m = shufflevector <16 x i32> undef, <16 x i32> %a, <16 x i32> <i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}
