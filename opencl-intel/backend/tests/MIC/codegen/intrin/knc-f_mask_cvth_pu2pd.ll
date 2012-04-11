; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.mask.cvth.pu2pd(<8 x double>, i8, <16 x i32>)

define <8 x double> @f_mask_cvth_pu2pd(<8 x double> %arg0, i8 %arg1, <16 x i32> %arg2) {
; KNF: f_mask_cvth_pu2pd:
; KNF: cvth
entry:
  %ret = call <8 x double> @llvm.x86.mic.mask.cvth.pu2pd(<8 x double> %arg0, i8 %arg1, <16 x i32> %arg2)

 ret <8 x double> %ret
}

