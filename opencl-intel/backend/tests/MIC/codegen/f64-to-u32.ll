; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define i32 @cvt(double %a) nounwind readnone ssp {
entry:
; KNF: vcvtpd2pu
  %conv = fptoui double %a to i32
  ret i32 %conv
}

@g = common global double 0.0, align 4

define i32 @cvtm() nounwind readnone ssp {
entry:
  %i = load double* @g
; KNF: vcvtpd2pu
  %conv = fptoui double %i to i32
  ret i32 %conv
}
