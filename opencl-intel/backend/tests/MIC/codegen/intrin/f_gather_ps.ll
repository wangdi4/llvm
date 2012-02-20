; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.gather.ps(<16 x i32>, i8 *, i32, i32, i32)

define <16 x float> @f_gather_ps(<16 x i32> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_gather_ps:
; KNF: vgatherps
entry:
  %ret = call <16 x float> @llvm.x86.mic.gather.ps(<16 x i32> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret <16 x float> %ret
}

