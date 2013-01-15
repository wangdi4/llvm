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

define <8 x double> @sub1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNC: sub1
; KNC: vsubpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %sub = fsub <8 x double> %a, %b
  ret <8 x double> %sub
}

define <8 x double> @sub2(<8 x double>* nocapture %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNC: sub2
; KNC: vsubrpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x double>* %a, align 64
  %sub = fsub <8 x double> %tmp1, %b
  ret <8 x double> %sub
}

define <8 x double> @sub3(<8 x double> %a, <8 x double>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: sub3
; KNC: vsubpd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp2 = load <8 x double>* %b, align 64
  %sub = fsub <8 x double> %a, %tmp2
  ret <8 x double> %sub
}

define <8 x double> @sub4(<8 x double> %a) nounwind readonly ssp {
entry:
; KNC: sub4
; KNC: vsubpd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; ret
  %tmp1 = load <8 x double>* @gb, align 64
  %sub = fsub <8 x double> %a, %tmp1
  ret <8 x double> %sub
}

define <8 x double> @sub5(<8 x double> %a) nounwind readonly ssp {
entry:
; KNC: sub5
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vsubpd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %sub = fsub <8 x double> %a, %tmp2
  ret <8 x double> %sub
}
