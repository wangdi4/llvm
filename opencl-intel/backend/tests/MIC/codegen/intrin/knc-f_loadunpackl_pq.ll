; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.loadunpackl.pq(<8 x i64>, i8 *, i32, i32)

define <8 x i64> @f_loadunpackl_pq(<8 x i64> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_loadunpackl_pq:
; KNF: vloadunpacklpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.loadunpackl.pq(<8 x i64> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3)

 ret <8 x i64> %ret
}

