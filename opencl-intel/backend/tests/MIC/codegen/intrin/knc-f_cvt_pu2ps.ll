; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.cvt.pu2ps(<16 x i32>, i32, i32)

define <16 x float> @f_cvt_pu2ps(<16 x i32> %arg0) {
; KNF: f_cvt_pu2ps:
; KNF: vcvtpu2ps $0, %v0, %v0
; KNC: f_cvt_pu2ps:
; KNC: vcvtfxpntudq2ps $0, %zmm0, %zmm0
entry:
  %ret = call <16 x float> @llvm.x86.mic.cvt.pu2ps(<16 x i32> %arg0, i32 4, i32 0)

 ret <16 x float> %ret
}

