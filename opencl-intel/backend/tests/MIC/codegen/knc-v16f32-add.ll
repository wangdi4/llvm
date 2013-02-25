; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8

define <16 x float> @add1(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNC: add1:
; KNC: vaddps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %add = fadd <16 x float> %a, %b
  ret <16 x float> %add
}

define <16 x float> @add2(<16 x float>* nocapture %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNC: add2:
; KNC: vaddps (%rdi), %zmm0, %zmm0
  %tmp1 = load <16 x float>* %a, align 64
  %add = fadd <16 x float> %tmp1, %b
  ret <16 x float> %add
}

define <16 x float> @add3(<16 x float> %a, <16 x float>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: add3:
; KNC: vaddps (%rdi), %zmm0, %zmm0
  %tmp2 = load <16 x float>* %b, align 64
  %add = fadd <16 x float> %tmp2, %a
  ret <16 x float> %add
}

define <16 x float> @add4(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: add4:
; KNC: vaddps  gb(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x float>* @gb, align 64
  %add = fadd <16 x float> %tmp1, %a
  ret <16 x float> %add
}

define <16 x float> @add5(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: add5:
; KNC: movq pgb(%rip), [[R1:%[a-z]+]]
; KNC: vaddps ([[R1]]), %zmm0, %zmm0
  %tmp1 = load <16 x float>** @pgb, align 8
  %tmp2 = load <16 x float>* %tmp1, align 64
  %add = fadd <16 x float> %tmp2, %a
  ret <16 x float> %add
}
