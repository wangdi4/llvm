; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.mskblend.pq(i8, <8 x i64>, <8 x i64>)

define <8 x i64> @f_mskblend_pq(i8 %arg0, <8 x i64> %arg1, <8 x i64> %arg2) {
; KNF: f_mskblend_pq:
; KNF: vmskblendpq
entry:
  %ret = call <8 x i64> @llvm.x86.mic.mskblend.pq(i8 %arg0, <8 x i64> %arg1, <8 x i64> %arg2)

 ret <8 x i64> %ret
}

