; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.round.ps(<16 x float>, i32, i32)

define <16 x float> @f_round_ps(<16 x float> %arg0, i32 %arg1, i32 %arg2) {
; KNF: vroundps $1, {rd}, {{%v[0-9]+}}, {{%v[0-9]+}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.round.ps(<16 x float> %arg0, i32 1, i32 1)

 ret <16 x float> %ret
}

