; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

define double @add(double %a, double %b) nounwind readnone ssp {
entry:
; KNC: vaddpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %add = fadd double %a, %b                       ; <double> [#uses=1]
  ret double %add
}
