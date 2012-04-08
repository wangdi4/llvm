; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.cvtl.ps2pd(<8 x double>, i8, <16 x float>)

define <8 x double> @f_mask_cvtl_ps2pd(<8 x double> %arg0, i8 %arg1, <16 x float> %arg2) {
; KNF: f_mask_cvtl_ps2pd:
; KNF: cvtl
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.cvtl.ps2pd(<8 x double> %arg0, i8 %arg1, <16 x float> %arg2)

 ret <8 x double> %ret
}

