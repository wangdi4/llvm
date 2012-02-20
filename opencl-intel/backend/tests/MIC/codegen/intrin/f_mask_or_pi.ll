; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32>, i16, <16 x i32>, <16 x i32>)

define <16 x i32> @f_mask_or_pi(<16 x i32> %arg0, i16 %arg1, <16 x i32> %arg2, <16 x i32> %arg3) {
; KNF: f_mask_or_pi:
; KNF: vorpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.or.pi(<16 x i32> %arg0, i16 %arg1, <16 x i32> %arg2, <16 x i32> %arg3)

 ret <16 x i32> %ret
}

