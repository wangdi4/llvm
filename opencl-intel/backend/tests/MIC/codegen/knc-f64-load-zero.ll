; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

define double @loadzero() nounwind readnone ssp {
entry:
;
; KNC: vpxorq    %zmm0, %zmm0, %zmm0

  ret double 0.000000e+00
}
