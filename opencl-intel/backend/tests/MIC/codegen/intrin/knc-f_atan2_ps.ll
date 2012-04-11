; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.atan2.ps(<16 x float>, <16 x float>)

define <16 x float> @f_atan2_ps(<16 x float> %arg0, <16 x float> %arg1) {
; KNF: f_atan2_ps:
; KNF: vatan2ps
entry:
  %ret = call <16 x float> @llvm.x86.mic.atan2.ps(<16 x float> %arg0, <16 x float> %arg1)

 ret <16 x float> %ret
}

