; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

target datalayout = "e-p:64:64"

define double @cvt(i8 %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2pd
; KNC: vcvtdq2pd
  %conv = uitofp i8 %a to double
  ret double %conv
}

define <16 x double> @cvt_vec(<16 x i8> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpu2pd
; KNC: vcvtudq2pd
  %conv = uitofp <16 x i8> %a to <16 x double>
  ret <16 x double> %conv
}

@g = common global i8 0, align 4

define double @cvtm() nounwind readnone ssp {
entry:
  %i = load i8* @g
; KNF: vcvtpi2pd
; KNC: vcvtdq2pd
  %conv = uitofp i8 %i to double
  ret double %conv
}
