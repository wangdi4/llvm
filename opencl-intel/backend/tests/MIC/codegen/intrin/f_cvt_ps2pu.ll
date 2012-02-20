; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.cvt.ps2pu(<16 x float>, i32, i32)

define <16 x i32> @f_cvt_ps2pu(<16 x float> %arg0, i32 %arg1, i32 %arg2) {
; KNF: f_cvt_ps2pu:
; KNF: cvt
entry:
  %ret = call <16 x i32> @llvm.x86.mic.cvt.ps2pu(<16 x float> %arg0, i32 %arg1, i32 %arg2)

 ret <16 x i32> %ret
}

