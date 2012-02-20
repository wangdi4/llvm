; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.loadunpackh.ps(<16 x float>, i8 *, i32, i32)

define <16 x float> @f_loadunpackh_ps(<16 x float> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_loadunpackh_ps:
; KNF: vloadunpackhps
entry:
  %ret = call <16 x float> @llvm.x86.mic.loadunpackh.ps(<16 x float> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3)

 ret <16 x float> %ret
}

