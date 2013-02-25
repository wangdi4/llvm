; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc -mattr=-fma-mic \
; RUN: | FileCheck %s --check-prefix=KNCNoFMA

;
;
;

target datalayout = "e-p:64:64"

define float @madd_bmcpa(float %a, float %b, float %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[132]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC:      vfmadd231ps %zmm1, %zmm2, %zmm0{%k1}
; KNCNoFMA: vmulps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNCNoFMA: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
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
;
; KNC:      vfmadd213ps %zmm1, %zmm2, %zmm0{%k1}
; KNCNoFMA: vmulps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNCNoFMA: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %mul = fmul float %c, %a
  %add = fadd float %b, %mul
  ret float %add
}
