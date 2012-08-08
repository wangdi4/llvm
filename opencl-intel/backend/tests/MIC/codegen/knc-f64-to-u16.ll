; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC
;

target datalayout = "e-p:64:64"

define i16 @cvt(double %a) nounwind readnone ssp {
entry:
; KNF: vcvtpd2pi
;
; KNC: vcvtfxpntpd2dq $3, %zmm0, %zmm{{[1-9]}}{%k{{[1-9]}}}
  %conv = fptoui double %a to i16
  ret i16 %conv
}

@g = common global double 0.0, align 4

define i16 @cvtm() nounwind readnone ssp {
entry:
  %i = load double* @g
; KNF: vcvtpd2pi
; 
; KNC: vcvtfxpntpd2dq $3, g(%rip){1to8}, %zmm0{%k{{[1-9]}}} 
  %conv = fptoui double %i to i16
  ret i16 %conv
}
