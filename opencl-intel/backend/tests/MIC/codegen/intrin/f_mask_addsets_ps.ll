; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.addsets.ps(<16 x float>, i16, <16 x float>, <16 x float>, i8 *)

define <16 x float> @f_mask_addsets_ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2, <16 x float> %arg3, i8 * %arg4) {
; KNF: f_mask_addsets_ps:
; KNF: vaddsetsps
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.addsets.ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2, <16 x float> %arg3, i8 * %arg4)

 ret <16 x float> %ret
}

