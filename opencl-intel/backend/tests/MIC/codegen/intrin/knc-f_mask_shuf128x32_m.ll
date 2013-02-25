; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.shuf128x32.m(<16 x float>, i16, i8 *, i32, i32)

define <16 x float> @f_mask_shuf128x32_m(<16 x float> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_mask_shuf128x32_m:
; KNF: shuf128x32
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.shuf128x32.m(<16 x float> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4)

 ret <16 x float> %ret
}

