; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.shuf128x32(<16 x float>, i32, i32)

define <16 x float> @f_shuf128x32(<16 x float> %arg0, i32 %arg1, i32 %arg2) {
; KNF: f_shuf128x32:
; KNF: shuf128x32
entry:
  %ret = call <16 x float> @llvm.x86.mic.shuf128x32(<16 x float> %arg0, i32 %arg1, i32 %arg2)

 ret <16 x float> %ret
}

