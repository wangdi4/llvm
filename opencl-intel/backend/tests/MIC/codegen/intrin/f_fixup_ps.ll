; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.fixup.ps(<16 x float>, <16 x float>, i32)

define <16 x float> @f_fixup_ps(<16 x float> %arg0, <16 x float> %arg1, i32 %arg2) {
; KNF: f_fixup_ps:
; KNF: vfixupps
entry:
  %ret = call <16 x float> @llvm.x86.mic.fixup.ps(<16 x float> %arg0, <16 x float> %arg1, i32 1001)

 ret <16 x float> %ret
}

