; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

define <16 x i32> @A(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; CHECK: .long 8
; CHECK: .long 9
; CHECK: .long 10
; CHECK: .long 11
; CHECK: .long 15
; CHECK: .long 14
; CHECK: .long 13
; CHECK: .long 12
; CHECK: .long 0
; CHECK: .long 0
; CHECK: .long 1
; CHECK: .long 7
; CHECK: .long 0
; CHECK: .long 9
; CHECK: .long 0
; CHECK: .long 0
; CHECK: vmovaps _const_{{[0-9]}}(%rip), [[R0:%zmm[0-9]+]]
; CHECK: vpermd %zmm0, [[R0]], %zmm0
  %m = shufflevector <16 x i32> %a, <16 x i32> %b, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 15, i32 14, i32 13, i32 12, i32 undef, i32 undef, i32 1, i32 7, i32 0, i32 9, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @B(<16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; CHECK: .long 8
; CHECK: .long 9
; CHECK: .long 10
; CHECK: .long 11
; CHECK: .long 15
; CHECK: .long 14
; CHECK: .long 13
; CHECK: .long 12
; CHECK: .long 0
; CHECK: .long 0
; CHECK: .long 1
; CHECK: .long 7
; CHECK: .long 0
; CHECK: .long 9
; CHECK: .long 0
; CHECK: .long 0
; CHECK: vmovaps _const_{{[0-9]}}(%rip), [[R0:%zmm[0-9]+]]
; CHECK: vpermd %zmm0, [[R0]], %zmm0
  %m = shufflevector <16 x float> %a, <16 x float> %b, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 15, i32 14, i32 13, i32 12, i32 undef, i32 undef, i32 1, i32 7, i32 0, i32 9, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <8 x i64> @C(<8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; CHECK: .long 8
; CHECK: .long 9
; CHECK: .long 12
; CHECK: .long 13
; CHECK: .long 14
; CHECK: .long 15
; CHECK: .long 12
; CHECK: .long 13
; CHECK: .long 0
; CHECK: .long 0
; CHECK: .long 2
; CHECK: .long 3
; CHECK: .long 0
; CHECK: .long 1
; CHECK: .long 0
; CHECK: .long 0
; CHECK: vmovaps _const_{{[0-9]}}(%rip), [[R0:%zmm[0-9]+]]
; CHECK: vpermd %zmm0, [[R0]], %zmm0
  %m = shufflevector <8 x i64> %a, <8 x i64> %b, <8 x i32> <i32 4, i32 6, i32 7, i32 6, i32 undef, i32 1, i32 0, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @D(<8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; CHECK: .long 8
; CHECK: .long 9
; CHECK: .long 12
; CHECK: .long 13
; CHECK: .long 14
; CHECK: .long 15
; CHECK: .long 12
; CHECK: .long 13
; CHECK: .long 0
; CHECK: .long 0
; CHECK: .long 2
; CHECK: .long 3
; CHECK: .long 0
; CHECK: .long 1
; CHECK: .long 0
; CHECK: .long 0
; CHECK: vmovaps _const_{{[0-9]}}(%rip), [[R0:%zmm[0-9]+]]
; CHECK: vpermd %zmm0, [[R0]], %zmm0
  %m = shufflevector <8 x double> %a, <8 x double> %b, <8 x i32> <i32 4, i32 6, i32 7, i32 6, i32 undef, i32 1, i32 0, i32 undef>
  ret <8 x double> %m
}
