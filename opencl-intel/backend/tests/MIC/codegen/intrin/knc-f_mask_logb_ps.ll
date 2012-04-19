; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.logb.ps(<16 x float>, i16, <16 x float>)

define <16 x float> @f_mask_logb_ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2) {
; KNF: f_mask_logb_ps:
; KNF: vlogbps
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.logb.ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2)

 ret <16 x float> %ret
}

