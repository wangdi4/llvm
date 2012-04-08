; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.broadcast.pq(i64)

define <8 x i64> @f_broadcast_pq(i64 %arg0) {
; KNF: f_broadcast_pq:
; KNF: vbroadcastpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.broadcast.pq(i64 %arg0)

 ret <8 x i64> %ret
}

