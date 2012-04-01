; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.cvt.pslo.pd(<16 x float>)

define <8 x double> @f_cvt_pslo_pd(<16 x float> %arg0) {
; KNF: f_cvt_pslo_pd:
; KNF: vcvtps2pd %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <8 x double> @llvm.x86.mic.cvt.pslo.pd(<16 x float> %arg0)

 ret <8 x double> %ret
}

