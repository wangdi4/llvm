; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.max.pu(<16 x i32>, <16 x i32>)

define <16 x i32> @f_max_pu(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNF: f_max_pu:
; KNF: vmaxpu
entry:
  %ret = call <16 x i32> @llvm.x86.mic.max.pu(<16 x i32> %arg0, <16 x i32> %arg1)

 ret <16 x i32> %ret
}

