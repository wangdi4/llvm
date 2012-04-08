; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.cdfnorminv.ps(<16 x float>)

define <16 x float> @f_cdfnorminv_ps(<16 x float> %arg0) {
; KNF: f_cdfnorminv_ps:
; KNF: vcdfnorminvps
entry:
  %ret = call <16 x float> @llvm.x86.mic.cdfnorminv.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

