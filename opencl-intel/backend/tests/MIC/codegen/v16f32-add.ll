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

define <16 x float> @add1(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %add = fadd <16 x float> %a, %b
  ret <16 x float> %add
}

define <16 x float> @add2(<16 x float>* nocapture %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNF: vaddps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vaddps {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* %a, align 64
  %add = fadd <16 x float> %tmp1, %b
  ret <16 x float> %add
}

define <16 x float> @add3(<16 x float> %a, <16 x float>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vaddps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vaddps {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x float>* %b, align 64
  %add = fadd <16 x float> %tmp2, %a
  ret <16 x float> %add
}

define <16 x float> @add4(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vaddps ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vaddps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @gb, align 64
  %add = fadd <16 x float> %tmp1, %a
  ret <16 x float> %add
}

define <16 x float> @add5(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vaddps ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vaddps ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>** @pgb, align 8
  %tmp2 = load <16 x float>* %tmp1, align 64
  %add = fadd <16 x float> %tmp2, %a
  ret <16 x float> %add
}
