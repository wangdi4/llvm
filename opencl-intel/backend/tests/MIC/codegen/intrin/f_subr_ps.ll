; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.subr.ps(<16 x float>, <16 x float>)

define <16 x float> @f_subr_ps(<16 x float> %arg0, <16 x float> %arg1) {
; KNF: f_subr_ps:
; KNF: vsubrps
entry:
  %ret = call <16 x float> @llvm.x86.mic.subr.ps(<16 x float> %arg0, <16 x float> %arg1)

 ret <16 x float> %ret
}

