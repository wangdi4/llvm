; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

target datalayout = "e-p:64:64"

define i8 @cvt(double %a) nounwind readnone ssp {
entry:
; KNF: vcvtpd2pi
;
; KNC: vcvtfxpntpd2dq $0, %zmm0, %zmm{{[1-9]}}{%k{{[1-9]}}}
  %conv = fptosi double %a to i8
  ret i8 %conv
}

@g = common global double 0.0, align 4

define i8 @cvtm() nounwind readnone ssp {
entry:
  %i = load double* @g
; KNF: vcvtpd2pi
; 
; KNC: vcvtfxpntpd2dq $0, g(%rip){1to8}, %zmm0{%k{{[1-9]}}} 
  %conv = fptosi double %i to i8
  ret i8 %conv
}
