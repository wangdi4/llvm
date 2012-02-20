; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare float @llvm.x86.mic.reduce.max.ps(<16 x float>)

define float @f_reduce_max_ps(<16 x float> %arg0) {
; KNF: f_reduce_max_ps:
; KNF: vreduceps
entry:
  %ret = call float @llvm.x86.mic.reduce.max.ps(<16 x float> %arg0)

 ret float %ret
}

