; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.cvt.pi2ps(<16 x float>, i16, <16 x i32>, i32, i32)

define <16 x float> @f_mask_cvt_pi2ps(<16 x float> %arg0, i16 %arg1, <16 x i32> %arg2) {
; KNF: f_mask_cvt_pi2ps:
; KNF: vcvtpi2ps
; KNC: f_mask_cvt_pi2ps:
; KNC: vcvtfxpntdq2ps
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.cvt.pi2ps(<16 x float> %arg0, i16 %arg1, <16 x i32> %arg2, i32 4, i32 0)

 ret <16 x float> %ret
}

