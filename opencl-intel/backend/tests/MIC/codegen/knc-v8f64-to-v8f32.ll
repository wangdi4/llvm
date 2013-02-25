; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define <8 x float> @d2f_trunc_vec(<8 x double> %v1) nounwind {
entry:
; KNC: vcvtpd2ps %zmm0, %zmm0
  %f1 = fptrunc <8 x double> %v1 to <8 x float>
  ret <8 x float> %f1
}
