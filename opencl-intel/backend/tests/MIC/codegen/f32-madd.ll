; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %s -check-prefix=KNFmpa 
;

target datalayout = "e-p:64:64"

define float @madd_bmcpa(float %a, float %b, float %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[132]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul float %b, %c
  %add = fadd float %a, %mul
  ret float %add
}

define float @madd_cmapb(float %a, float %b, float %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[132]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul float %c, %a
  %add = fadd float %b, %mul
  ret float %add
}
