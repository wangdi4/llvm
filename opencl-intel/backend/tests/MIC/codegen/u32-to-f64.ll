; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"


define double @cvt(i32 %a) nounwind readnone ssp {
entry:
; KNF: vcvtpu2pd
  %conv = uitofp i32 %a to double
  ret double %conv
}

define <8 x double> @cvt_vec(<8 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpu2pd
  %conv = uitofp <8 x i32> %a to <8 x double>
  ret <8 x double> %conv
}

@g = common global i32 0, align 4

define double @cvtm() nounwind readnone ssp {
entry:
  %i = load i32* @g
; KNF: vcvtpu2pd
  %conv = uitofp i32 %i to double
  ret double %conv
}
