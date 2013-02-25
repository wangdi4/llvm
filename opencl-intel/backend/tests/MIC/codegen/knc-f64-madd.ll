; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc -mattr=-fma-mic \
; RUN: | FileCheck %s --check-prefix=KNCNoFMA

target datalayout = "e-p:64:64"

define double @madd_ambpc(double %a, double %b, double %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[132]+pd}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vfmadd213pd %zmm2, %zmm1, %zmm0{%k1}
  %mul = fmul double %a, %b                       ; <double> [#uses=1]
  %add = fadd double %mul, %c                       ; <double> [#uses=1]
  ret double %add
}

define double @madd_bmcpa(double %a, double %b, double %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[132]+pd}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vfmadd231pd %zmm1, %zmm2, %zmm0{%k1}
;
; KNCNoFMA: vmulpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNCNoFMA: vaddpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %mul = fmul double %b, %c                       ; <double> [#uses=1]
  %add = fadd double %mul, %a                       ; <double> [#uses=1]
  ret double %add
}
