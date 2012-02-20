; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.upconv.float64(i8 *, i32, i32)

define <8 x double> @f_upconv_float64(i8 * %arg0, i32 %arg1, i32 %arg2) {
; KNF: f_upconv_float64:
; KNF: upconv
entry:
  %ret = call <8 x double> @llvm.x86.mic.upconv.float64(i8 * %arg0, i32 %arg1, i32 %arg2)

 ret <8 x double> %ret
}

