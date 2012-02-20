; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.cvtins.ps2f11(<16 x float>, <16 x float>, i32, i32)

define <16 x float> @f_cvtins_ps2f11(<16 x float> %arg0, <16 x float> %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_cvtins_ps2f11:
; KNF: cvtins
entry:
  %ret = call <16 x float> @llvm.x86.mic.cvtins.ps2f11(<16 x float> %arg0, <16 x float> %arg1, i32 %arg2, i32 %arg3)

 ret <16 x float> %ret
}

