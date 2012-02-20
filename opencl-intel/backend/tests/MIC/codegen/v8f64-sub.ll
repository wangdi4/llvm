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

define <8 x double> @sub1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNF: vsubpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <8 x double> %a, %b
  ret <8 x double> %sub
}

define <8 x double> @sub2(<8 x double>* nocapture %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNF: vsubrpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubrpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>* %a, align 64
  %sub = fsub <8 x double> %tmp1, %b
  ret <8 x double> %sub
}

define <8 x double> @sub3(<8 x double> %a, <8 x double>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsubpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <8 x double>* %b, align 64
  %sub = fsub <8 x double> %a, %tmp2
  ret <8 x double> %sub
}

define <8 x double> @sub4(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vsubpd ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vsubpd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>* @gb, align 64
  %sub = fsub <8 x double> %a, %tmp1
  ret <8 x double> %sub
}

define <8 x double> @sub5(<8 x double> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vsubpd ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vsubpd ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %sub = fsub <8 x double> %a, %tmp2
  ret <8 x double> %sub
}
