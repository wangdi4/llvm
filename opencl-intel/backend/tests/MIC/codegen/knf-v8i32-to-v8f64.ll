; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <8 x double> @cvt_vec(<8 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2pd $0, %v0, %v0
  %conv = sitofp <8 x i32> %a to <8 x double>
  ret <8 x double> %conv
}
