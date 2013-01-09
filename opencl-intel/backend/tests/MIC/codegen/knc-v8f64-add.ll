; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <8 x double> zeroinitializer, align 64
@pgb = common global <8 x double>* null, align 8

define <8 x double> @add1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNC: add1:
; KNC: vaddpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %add = fadd <8 x double> %a, %b
  ret <8 x double> %add
}

define <8 x double> @add2(<8 x double>* nocapture %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNC: add2:
; KNC: vaddpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x double>* %a, align 64
  %add = fadd <8 x double> %tmp1, %b
  ret <8 x double> %add
}

define <8 x double> @add3(<8 x double> %a, <8 x double>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: add3:
; KNC: vaddpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp2 = load <8 x double>* %b, align 64
  %add = fadd <8 x double> %tmp2, %a
  ret <8 x double> %add
}

define <8 x double> @add4(<8 x double> %a) nounwind readonly ssp {
entry:
; KNC: add4:
; KNC: vaddpd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x double>* @gb, align 64
  %add = fadd <8 x double> %tmp1, %a
  ret <8 x double> %add
}

define <8 x double> @add5(<8 x double> %a) nounwind readonly ssp {
entry:
; KNC: add5:
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vaddpd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %add = fadd <8 x double> %tmp2, %a
  ret <8 x double> %add
}
