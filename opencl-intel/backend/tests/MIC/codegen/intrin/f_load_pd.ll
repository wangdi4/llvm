; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.load.pd(i8 *, i32, i32, i32)

define <8 x double> @f_load_pd(i8 * %arg0) {
; KNF: f_load_pd:
; KNF: vloadq (%{{[a-z]+}}){4to8}
entry:
  %ret = call <8 x double> @llvm.x86.mic.load.pd(i8 * %arg0, i32 0, i32 2, i32 0)

 ret <8 x double> %ret
}

