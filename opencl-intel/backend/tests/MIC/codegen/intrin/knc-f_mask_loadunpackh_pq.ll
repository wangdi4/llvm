; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64>, i8, i8 *, i32, i32)

define <8 x i64> @f_mask_loadunpackh_pq(<8 x i64> %arg0, i8 %arg1, i8 * %arg2) {
; KNF: f_mask_loadunpackh_pq:
; KNF: vloadunpackhq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.mask.loadunpackh.pq(<8 x i64> %arg0, i8 %arg1, i8 * %arg2, i32 0, i32 0)

 ret <8 x i64> %ret
}

