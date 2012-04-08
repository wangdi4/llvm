; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float>, i32)

define <16 x float> @f_swizzle_ps(<16 x float> %arg0, i32 %arg1) {
; KNF: f_swizzle_ps:
; KNF: vswizzleps
entry:
  %ret = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %arg0, i32 %arg1)

 ret <16 x float> %ret
}

