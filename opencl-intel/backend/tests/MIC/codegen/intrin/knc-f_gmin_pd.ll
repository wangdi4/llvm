; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.gmin.pd(<8 x double>, <8 x double>)

define <8 x double> @f_gmin_pd(<8 x double> %arg0, <8 x double> %arg1) {
; KNF: f_gmin_pd:
; KNF: vgminpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.gmin.pd(<8 x double> %arg0, <8 x double> %arg1)

 ret <8 x double> %ret
}

