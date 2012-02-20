; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.mask.cmpnle.ps(i16, <16 x float>, <16 x float>)

define i16 @f_mask_cmpnle_ps(i16 %arg0, <16 x float> %arg1, <16 x float> %arg2) {
; KNF: f_mask_cmpnle_ps:
; KNF: vcmpps
entry:
  %ret = call i16 @llvm.x86.mic.mask.cmpnle.ps(i16 %arg0, <16 x float> %arg1, <16 x float> %arg2)

 ret i16 %ret
}

