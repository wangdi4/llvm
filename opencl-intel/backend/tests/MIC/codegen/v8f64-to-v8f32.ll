; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <8 x float> @d2f_trunc_vec(<8 x double> %v1) nounwind {
entry:
; KNF: vcvtpd2ps $0, {rn}, %v0, %v0{%k1}
  %f1 = fptrunc <8 x double> %v1 to <8 x float>
  ret <8 x float> %f1
}
