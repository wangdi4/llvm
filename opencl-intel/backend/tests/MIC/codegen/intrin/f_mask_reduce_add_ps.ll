; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare float @llvm.x86.mic.mask.reduce.add.ps(i16, <16 x float>)

define float @f_mask_reduce_add_ps(i16 %arg0, <16 x float> %arg1) {
; KNF: f_mask_reduce_add_ps:
; KNF: vreduceps
entry:
  %ret = call float @llvm.x86.mic.mask.reduce.add.ps(i16 %arg0, <16 x float> %arg1)

 ret float %ret
}

