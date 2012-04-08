; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.cvth.pd2pu(<16 x i32>, <8 x double>, i32)

define <16 x i32> @f_cvth_pd2pu(<16 x i32> %arg0, <8 x double> %arg1, i32 %arg2) {
; KNF: f_cvth_pd2pu:
; KNF: cvth
entry:
  %ret = call <16 x i32> @llvm.x86.mic.cvth.pd2pu(<16 x i32> %arg0, <8 x double> %arg1, i32 %arg2)

 ret <16 x i32> %ret
}

