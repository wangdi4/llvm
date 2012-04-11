; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32>, i16, i8 *, i32, i32)

define <16 x i32> @f_mask_loadunpackl_pi(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_mask_loadunpackl_pi:
; KNF: vloadunpacklpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.loadunpackl.pi(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4)

 ret <16 x i32> %ret
}

