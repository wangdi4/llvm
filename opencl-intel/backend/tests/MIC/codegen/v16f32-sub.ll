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

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8

define <16 x float> @sub1(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vsubps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <16 x float> %a, %b
  ret <16 x float> %sub
}

define <16 x float> @sub2(<16 x float>* nocapture %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNF: vsubrps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubrps {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* %a, align 64
  %sub = fsub <16 x float> %tmp1, %b
  ret <16 x float> %sub
}

define <16 x float> @sub3(<16 x float> %a, <16 x float>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsubps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vsubps {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x float>* %b, align 64
  %sub = fsub <16 x float> %a, %tmp2
  ret <16 x float> %sub
}

define <16 x float> @sub4(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vsubps ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vsubps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @gb, align 64
  %sub = fsub <16 x float> %a, %tmp1
  ret <16 x float> %sub
}

define <16 x float> @sub5(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vsubps ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vsubps ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>** @pgb, align 8
  %tmp2 = load <16 x float>* %tmp1, align 64
  %sub = fsub <16 x float> %a, %tmp2
  ret <16 x float> %sub
}
