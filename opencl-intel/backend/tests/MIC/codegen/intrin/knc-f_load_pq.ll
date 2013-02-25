; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.load.pq(i8 *, i32, i32, i32)

define <8 x i64> @f_load_pq(i8 * %arg0) {
; KNF: f_load_pq:
; KNF: vloadq (%{{[a-z]+}}){1to8}{nt}
entry:
  %ret = call <8 x i64> @llvm.x86.mic.load.pq(i8 * %arg0, i32 0, i32 1, i32 1)

 ret <8 x i64> %ret
}

