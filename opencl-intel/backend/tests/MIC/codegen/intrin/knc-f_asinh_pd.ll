; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.asinh.pd(<8 x double>)

define <8 x double> @f_asinh_pd(<8 x double> %arg0) {
; KNF: f_asinh_pd:
; KNF: vasinhpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.asinh.pd(<8 x double> %arg0)

 ret <8 x double> %ret
}

