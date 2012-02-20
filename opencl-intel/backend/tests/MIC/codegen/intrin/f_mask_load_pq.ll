; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.mask.load.pq(<8 x i64>, i8, i8 *, i32, i32, i32)

define <8 x i64> @f_mask_load_pq(<8 x i64> %arg0, i8 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_mask_load_pq:
; KNF: vloadpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.mask.load.pq(<8 x i64> %arg0, i8 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret <8 x i64> %ret
}

