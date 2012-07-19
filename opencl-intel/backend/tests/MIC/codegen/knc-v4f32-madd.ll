; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc -mattr=-fma-mic \
; RUN: | FileCheck %s --check-prefix=KNCNoFMA


target datalayout = "e-p:64:64"

define <4 x float> @madd_cmbpa(<4 x float> %a, <4 x float> %b, <4 x float> %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vfmadd231ps %zmm2, %zmm1, %zmm0
; KNCNoFMA: vmulps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNCNoFMA: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}

  %mul = fmul <4 x float> %c, %b
  %add = fadd <4 x float> %a, %mul
  ret <4 x float> %add
}

define <4 x float> @madd_ambpc(<4 x float> %a, <4 x float> %b, <4 x float> %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vfmadd213ps %zmm2, %zmm1, %zmm0
; KNCNoFMA: vmulps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNCNoFMA: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %mul = fmul <4 x float> %a, %b
  %add = fadd <4 x float> %c, %mul
  ret <4 x float> %add
}

