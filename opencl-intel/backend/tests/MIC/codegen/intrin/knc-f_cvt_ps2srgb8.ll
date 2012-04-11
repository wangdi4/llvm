; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.cvt.ps2srgb8(<16 x float>)

define <16 x i32> @f_cvt_ps2srgb8(<16 x float> %arg0) {
; KNF: f_cvt_ps2srgb8:
; KNF: cvt
entry:
  %ret = call <16 x i32> @llvm.x86.mic.cvt.ps2srgb8(<16 x float> %arg0)

 ret <16 x i32> %ret
}

