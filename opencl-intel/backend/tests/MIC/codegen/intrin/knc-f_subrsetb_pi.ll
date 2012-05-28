; XFAIL: win32

; RUN: echo
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.subrsetb.pi(<16 x i32>, <16 x i32>, i8 *)

define <16 x i32> @f_subrsetb_pi(<16 x i32> %arg0, <16 x i32> %arg1, i8 * %arg2) {
; KNF: f_subrsetb_pi:
; KNF: vsubrsetbpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.subrsetb.pi(<16 x i32> %arg0, <16 x i32> %arg1, i8 * %arg2)

 ret <16 x i32> %ret
}

