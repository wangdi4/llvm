; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.cvt.ps2srgb8(<16 x i32>, i16, <16 x float>)

define <16 x i32> @f_mask_cvt_ps2srgb8(<16 x i32> %arg0, i16 %arg1, <16 x float> %arg2) {
; KNF: f_mask_cvt_ps2srgb8:
; KNF: cvt
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.cvt.ps2srgb8(<16 x i32> %arg0, i16 %arg1, <16 x float> %arg2)

 ret <16 x i32> %ret
}

