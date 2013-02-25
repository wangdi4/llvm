; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.maddn213.ps(<16 x float>, <16 x float>, <16 x float>)

define <16 x float> @f_maddn213_ps(<16 x float> %arg0, <16 x float> %arg1, <16 x float> %arg2) {
; KNF: f_maddn213_ps:
; KNF: vmaddn213ps
entry:
  %ret = call <16 x float> @llvm.x86.mic.maddn213.ps(<16 x float> %arg0, <16 x float> %arg1, <16 x float> %arg2)

 ret <16 x float> %ret
}

