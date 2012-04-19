; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.scale.ps(<16 x float>, <16 x i32>)

define <16 x float> @f_scale_ps(<16 x float> %arg0, <16 x i32> %arg1) {
; KNF: f_scale_ps:
; KNF: vscaleps
entry:
  %ret = call <16 x float> @llvm.x86.mic.scale.ps(<16 x float> %arg0, <16 x i32> %arg1)

 ret <16 x float> %ret
}

