; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.gather.pq(<8 x i64>, i8 *, i32, i32, i32)

define <8 x i64> @f_gather_pq(<8 x i64> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_gather_pq:
; KNF: vgatherpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.gather.pq(<8 x i64> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret <8 x i64> %ret
}

