; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.and.pq(<8 x i64>, <8 x i64>)

define <8 x i64> @f_and_pq(<8 x i64> %arg0, <8 x i64> %arg1) {
; KNF: f_and_pq:
; KNF: vandpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.and.pq(<8 x i64> %arg0, <8 x i64> %arg1)

 ret <8 x i64> %ret
}

