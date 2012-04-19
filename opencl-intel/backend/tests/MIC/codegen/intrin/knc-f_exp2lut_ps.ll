; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.exp2lut.ps(<16 x i32>)

define <16 x float> @f_exp2lut_ps(<16 x i32> %arg0) {
; KNF: f_exp2lut_ps:
; KNF: vexp2lutps
entry:
  %ret = call <16 x float> @llvm.x86.mic.exp2lut.ps(<16 x i32> %arg0)

 ret <16 x float> %ret
}

