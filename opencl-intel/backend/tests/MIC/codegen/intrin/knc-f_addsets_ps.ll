; XFAIL: win32

; RUN: echo
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.addsets.ps(<16 x float>, <16 x float>, i8 *)

define <16 x float> @f_addsets_ps(<16 x float> %arg0, <16 x float> %arg1, i8 * %arg2) {
; KNF: f_addsets_ps:
; KNF: vaddsetsps
entry:
  %ret = call <16 x float> @llvm.x86.mic.addsets.ps(<16 x float> %arg0, <16 x float> %arg1, i8 * %arg2)

 ret <16 x float> %ret
}

