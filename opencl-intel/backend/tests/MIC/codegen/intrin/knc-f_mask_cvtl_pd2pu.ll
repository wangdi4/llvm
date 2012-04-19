; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.cvtl.pd2pu(<16 x i32>, i8, <8 x double>, i32)

define <16 x i32> @f_mask_cvtl_pd2pu(<16 x i32> %arg0, i8 %arg1, <8 x double> %arg2, i32 %arg3) {
; KNF: f_mask_cvtl_pd2pu:
; KNF: cvtl
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.cvtl.pd2pu(<16 x i32> %arg0, i8 %arg1, <8 x double> %arg2, i32 %arg3)

 ret <16 x i32> %ret
}

