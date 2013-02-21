; XFAIL: win32


; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.cos.pd(<8 x double>, i8, <8 x double>)

define <8 x double> @f_mask_cos_pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2) {
; KNF: f_mask_cos_pd:
; KNF: vcospd
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.cos.pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2)

 ret <8 x double> %ret
}

