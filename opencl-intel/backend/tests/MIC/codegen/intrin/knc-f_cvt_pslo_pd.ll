; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.cvt.pslo.pd(<16 x float>)

define <8 x double> @f_cvt_pslo_pd(<16 x float> %arg0) {
; KNF: f_cvt_pslo_pd:
; KNF: vcvtps2pd $0, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNC: f_cvt_pslo_pd:
; KNC: vcvtps2pd %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <8 x double> @llvm.x86.mic.cvt.pslo.pd(<16 x float> %arg0)

 ret <8 x double> %ret
}


declare <8 x double> @llvm.x86.mic.mask.cvtl.ps2pd(<8 x double>, i8, <16 x float>) nounwind readnone

define <8 x double> @test_1(<8 x double> %arg1, <16 x float> %arg2) {
; KNF: vcvtps2pd
; KNC: vcvtps2pd
  %retVal = call <8 x double> @llvm.x86.mic.mask.cvtl.ps2pd(<8 x double> %arg1, i8 1, <16 x float> %arg2) nounwind
  ret <8 x double> %retVal
}
