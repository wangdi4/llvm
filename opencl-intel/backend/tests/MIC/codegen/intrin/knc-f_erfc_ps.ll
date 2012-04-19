; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.erfc.ps(<16 x float>)

define <16 x float> @f_erfc_ps(<16 x float> %arg0) {
; KNF: f_erfc_ps:
; KNF: verfcps
entry:
  %ret = call <16 x float> @llvm.x86.mic.erfc.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

