; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

define float @sub(float %a, float %b) nounwind readnone ssp {
entry:
; KNF: vsubps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub float %a, %b                        ; <float> [#uses=1]
  ret float %sub
}
