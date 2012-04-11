; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define <8 x i32> @cvt_vec(<8 x double> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpd2pi
  %conv = fptosi <8 x double> %a to <8 x i32>
  ret <8 x i32> %conv
}
