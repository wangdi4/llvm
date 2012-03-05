; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.swizzle.pq(<8 x i64>, i32)

define <8 x i64> @f_swizzle_pq(<8 x i64> %arg0) {
; KNF: f_swizzle_pq:
; KNF: vshuf128x32 $68, $160
entry:
  %ret = call <8 x i64> @llvm.x86.mic.swizzle.pq(<8 x i64> %arg0, i32 3)

 ret <8 x i64> %ret
}

