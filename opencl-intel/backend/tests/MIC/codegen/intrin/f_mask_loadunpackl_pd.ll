; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double>, i8, i8 *, i32, i32)

define <8 x double> @f_mask_loadunpackl_pd(<8 x double> %arg0, i8 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_mask_loadunpackl_pd:
; KNF: vloadunpacklpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.loadunpackl.pd(<8 x double> %arg0, i8 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4)

 ret <8 x double> %ret
}

