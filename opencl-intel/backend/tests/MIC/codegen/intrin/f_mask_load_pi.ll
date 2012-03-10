; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.load.pi(<16 x i32>, i16, i8 *, i32, i32, i32)

define <16 x i32> @f_mask_load_pi(<16 x i32> %arg0, i16 %arg1, i8 * %arg2) {
; KNF: f_mask_load_pi:
; KNF: vloadd (%{{[a-z]+}}), %v{{[0-9]*}}{%k{{[0-9]*}}}
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.load.pi(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 0, i32 0, i32 0)

 ret <16 x i32> %ret
}

