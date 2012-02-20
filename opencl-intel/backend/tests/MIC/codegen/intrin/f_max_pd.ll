; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.max.pd(<8 x double>, <8 x double>)

define <8 x double> @f_max_pd(<8 x double> %arg0, <8 x double> %arg1) {
; KNF: f_max_pd:
; KNF: vmaxpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.max.pd(<8 x double> %arg0, <8 x double> %arg1)

 ret <8 x double> %ret
}

