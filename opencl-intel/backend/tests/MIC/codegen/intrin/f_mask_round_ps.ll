; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.round.ps(<16 x float>, i16, <16 x float>, i32, i32)

define <16 x float> @f_mask_round_ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2, i32 %arg3, i32 %arg4) {
; KNF: vroundps $1, {ru}, {{%v[0-9]+}}, {{%v[0-9]+}}



entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.round.ps(<16 x float> %arg0, i16 1, <16 x float> %arg2, i32 2, i32 1)

 ret <16 x float> %ret
}

