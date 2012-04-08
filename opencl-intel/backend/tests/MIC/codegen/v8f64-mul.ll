; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <8 x double> zeroinitializer, align 64
@pgb = common global <8 x double>* null, align 8

define <8 x double> @mul1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNF: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vmulpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %mul = fmul <8 x double> %a, %b
  ret <8 x double> %mul
}

define <8 x double> @mul2(<8 x double>* nocapture %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNF: vmulpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vmulpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>* %a, align 64
  %mul = fmul <8 x double> %tmp1, %b
  ret <8 x double> %mul
}

define <8 x double> @mul3(<8 x double> %a, <8 x double>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vmulpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vmulpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <8 x double>* %b, align 64
  %mul = fmul <8 x double> %tmp2, %a
  ret <8 x double> %mul
}

define <8 x double> @mul4(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vmulpd ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vmulpd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>* @gb, align 64
  %mul = fmul <8 x double> %tmp1, %a
  ret <8 x double> %mul
}

define <8 x double> @mul5(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vmulpd ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vmulpd ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %mul = fmul <8 x double> %tmp2, %a
  ret <8 x double> %mul
}
