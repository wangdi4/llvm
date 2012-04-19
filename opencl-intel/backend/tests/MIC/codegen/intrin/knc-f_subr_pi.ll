; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.subr.pi(<16 x i32>, <16 x i32>)

define <16 x i32> @f_subr_pi(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNF: f_subr_pi:
; KNF: vsubrpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.subr.pi(<16 x i32> %arg0, <16 x i32> %arg1)

 ret <16 x i32> %ret
}

