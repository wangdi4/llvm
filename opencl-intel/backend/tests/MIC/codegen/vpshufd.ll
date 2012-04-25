; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc     \
; RUN: | FileCheck %s

define <16 x i32> @A(<16 x i32> %a) nounwind ssp {
entry:
; CHECK: vpshufd $225, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 1, i32 0, i32 2, i32 3, i32 5, i32 4, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x i32> @D(<16 x i32> %a) nounwind ssp {
entry:
; CHECK: vpshufd $216, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 8, i32 10, i32 9, i32 11, i32 undef, i32 14, i32 13, i32 15>
  ret <16 x i32> %m
}

define <16 x i32> @E(<16 x i32> %a) nounwind ssp {
entry:
; CHECK: vpshufd $12, %zmm0, %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> undef, <16 x i32> <i32 undef, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 12>
  ret <16 x i32> %m
}
