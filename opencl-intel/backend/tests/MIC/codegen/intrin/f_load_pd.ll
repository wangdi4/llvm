; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.load.pd(i8 *, i32, i32, i32)

define <8 x double> @f_load_pd(i8 * %arg0, i32 %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_load_pd:
; KNF: vloadpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.load.pd(i8 * %arg0, i32 %arg1, i32 %arg2, i32 %arg3)

 ret <8 x double> %ret
}

