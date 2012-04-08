; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.floor.pd(<8 x double>)

define <8 x double> @f_floor_pd(<8 x double> %arg0) {
; KNF: f_floor_pd:
; KNF: vfloorpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.floor.pd(<8 x double> %arg0)

 ret <8 x double> %ret
}

