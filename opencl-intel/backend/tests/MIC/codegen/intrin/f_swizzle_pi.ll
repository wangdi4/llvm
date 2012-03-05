; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.swizzle.pi(<16 x i32>, i32)

define <16 x i32> @f_swizzle_pi(<16 x i32> %arg0) {
; KNF: f_swizzle_pi:
; KNF: vshuf128x32 $177, $228

entry:
  %ret = call <16 x i32> @llvm.x86.mic.swizzle.pi(<16 x i32> %arg0, i32 1)

 ret <16 x i32> %ret
}

