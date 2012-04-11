; XFAIL: win32
; XFAIL: *
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;


; RUN: cat files_that_does_not_exist
; guyblank: test gets stuck on KNC, added the previous line so that it won't
; guyblank: be marked as unresolved

target datalayout = "e-p:64:64"

define <8 x float> @d2f_trunc_vec(<8 x double> %v1) nounwind {
entry:
; KNF: vcvtpd2ps $0, {rn}, %v0, %v0{%k1}
  %f1 = fptrunc <8 x double> %v1 to <8 x float>
  ret <8 x float> %f1
}
