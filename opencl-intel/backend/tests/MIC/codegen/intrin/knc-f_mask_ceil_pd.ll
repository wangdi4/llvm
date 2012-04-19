; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.ceil.pd(<8 x double>, i8, <8 x double>)

define <8 x double> @f_mask_ceil_pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2) {
; KNF: f_mask_ceil_pd:
; KNF: vceilpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.ceil.pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2)

 ret <8 x double> %ret
}

