; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.atanh.ps(<16 x float>)

define <16 x float> @f_atanh_ps(<16 x float> %arg0) {
; KNF: f_atanh_ps:
; KNF: vatanhps
entry:
  %ret = call <16 x float> @llvm.x86.mic.atanh.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

