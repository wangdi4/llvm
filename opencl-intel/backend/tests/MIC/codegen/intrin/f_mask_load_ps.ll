; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float>, i16, i8 *, i32, i32, i32)

define <16 x float> @f_mask_load_ps(<16 x float> %arg0, i16 %arg1, i8 * %arg2) {
; KNF: f_mask_load_ps:
; KNF: vloadd (%{{[a-z]+}}), %v{{[0-9]*}}{%k{{[0-9]*}}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.load.ps(<16 x float> %arg0, i16 %arg1, i8 * %arg2, i32 0, i32 0, i32 0)

 ret <16 x float> %ret
}

