; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.gmax.pd(<8 x double>, i8, <8 x double>, <8 x double>)

define <8 x double> @f_mask_gmax_pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2, <8 x double> %arg3) {
; KNF: f_mask_gmax_pd:
; KNF: vgmaxpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.gmax.pd(<8 x double> %arg0, i8 %arg1, <8 x double> %arg2, <8 x double> %arg3)

 ret <8 x double> %ret
}

