; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.loadunpackl.ps(<16 x float>, i8 *, i32, i32)

define <16 x float> @f_loadunpackl_ps(<16 x float> %arg0, i8 * %arg1) {
; KNF: f_loadunpackl_ps:
; KNF: vloadunpackld (%{{[a-z]*}}){float16}{nt}, %v{{[0-9]*}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.loadunpackl.ps(<16 x float> %arg0, i8 * %arg1, i32 1, i32 1)

 ret <16 x float> %ret
}

