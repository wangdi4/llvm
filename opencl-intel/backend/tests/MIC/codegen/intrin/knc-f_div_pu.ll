; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.div.pu(<16 x i32>, <16 x i32>)

define <16 x i32> @f_div_pu(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNF: f_div_pu:
; KNF: vdivpu
entry:
  %ret = call <16 x i32> @llvm.x86.mic.div.pu(<16 x i32> %arg0, <16 x i32> %arg1)

 ret <16 x i32> %ret
}

