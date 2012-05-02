; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 


target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double>, i8, i8 *, i32, i32)

define <8 x double> @f_mask_loadunpackl_pd(<8 x double> %arg0, i8 %arg1, i8 * %arg2) {
; KNF: f_mask_loadunpackl_pd:
; KNF:  vkmov     %{{[a-z]*}}, %k{{[0-9]*}}
; KNF:  vloadunpacklq (%{{[a-z]*}}){nt}, %v{{[0-9]*}}{%k{{[0-9]*}}}

  %ret = call <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double> %arg0, i8 %arg1, i8 * %arg2, i32 0, i32 1)

 ret <8 x double> %ret
}

