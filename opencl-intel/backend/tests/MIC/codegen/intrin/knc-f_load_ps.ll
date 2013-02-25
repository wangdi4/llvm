; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 


target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.load.ps(i8 *, i32, i32, i32)

define <16 x float> @f_load_ps(i8 * %arg0) {
; KNF: f_load_ps:
; KNF: vloadd (%{{[a-z]+}}){sint16}{1to16}{nt}
entry:
; 5 = full up conversion from sint16, 1 = broadcast 1to16 , 1 = non-temporal
  %ret = call <16 x float> @llvm.x86.mic.load.ps(i8 * %arg0, i32 5, i32 1, i32 1)
  ret <16 x float> %ret
}

