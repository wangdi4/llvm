; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

define <8 x double> @loadzero() nounwind readnone ssp {
entry:
; CHECK: vpxorq %zmm0, %zmm0, %zmm0
  ret <8 x double> zeroinitializer
}
