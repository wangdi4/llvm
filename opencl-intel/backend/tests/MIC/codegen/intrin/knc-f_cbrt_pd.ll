; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.cbrt.pd(<8 x double>)

define <8 x double> @f_cbrt_pd(<8 x double> %arg0) {
; KNF: f_cbrt_pd:
; KNF: vcbrtpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.cbrt.pd(<8 x double> %arg0)

 ret <8 x double> %ret
}

