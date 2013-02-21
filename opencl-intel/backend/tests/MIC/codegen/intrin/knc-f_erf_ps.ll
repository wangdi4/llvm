; XFAIL: win32


; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.erf.ps(<16 x float>)

define <16 x float> @f_erf_ps(<16 x float> %arg0) {
; KNF: f_erf_ps:
; KNF: verfps
entry:
  %ret = call <16 x float> @llvm.x86.mic.erf.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

