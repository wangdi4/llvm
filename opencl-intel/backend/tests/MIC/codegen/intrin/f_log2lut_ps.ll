; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.log2lut.ps(<16 x float>)

define <16 x float> @f_log2lut_ps(<16 x float> %arg0) {
; KNF: f_log2lut_ps:
; KNF: vlog2lutps
entry:
  %ret = call <16 x float> @llvm.x86.mic.log2lut.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

