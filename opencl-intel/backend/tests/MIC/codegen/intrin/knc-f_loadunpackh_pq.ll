; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.loadunpackh.pq(<8 x i64>, i8 *, i32, i32)

define <8 x i64> @f_loadunpackh_pq(<8 x i64> %arg0, i8 * %arg1) {
; KNF: f_loadunpackh_pq:
; KNF: 
entry:
  %ret = call <8 x i64> @llvm.x86.mic.loadunpackh.pq(<8 x i64> %arg0, i8 * %arg1, i32 0, i32 0)

 ret <8 x i64> %ret
}

