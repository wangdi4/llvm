; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.mask.cmpnlt.pi(i16, <16 x i32>, <16 x i32>)

define i16 @f_mask_cmpnlt_pi(i16 %arg0, <16 x i32> %arg1, <16 x i32> %arg2) {
; KNF: f_mask_cmpnlt_pi:
; KNF: vcmppi
entry:
  %ret = call i16 @llvm.x86.mic.mask.cmpnlt.pi(i16 %arg0, <16 x i32> %arg1, <16 x i32> %arg2)

 ret i16 %ret
}

