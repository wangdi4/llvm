; XFAIL: win32
; XFAIL: *
; 
; REVIEW: Once this compiles, we need to add KNF: lines with the
;         expected output.
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <8 x float> zeroinitializer, align 32
@pgb = common global <8 x float>* null, align 8

define <8 x float> @add1(<8 x float> %a, <8 x float> %b) nounwind readnone ssp {
entry:
  %add = fadd <8 x float> %a, %b
  ret <8 x float> %add
}

define <8 x float> @add2(<8 x float>* nocapture %a, <8 x float> %b) nounwind readonly ssp {
entry:
  %tmp1 = load <8 x float>* %a, align 32
  %add = fadd <8 x float> %tmp1, %b
  ret <8 x float> %add
}

define <8 x float> @add3(<8 x float> %a, <8 x float>* nocapture %b) nounwind readonly ssp {
entry:
  %tmp2 = load <8 x float>* %b, align 32
  %add = fadd <8 x float> %tmp2, %a
  ret <8 x float> %add
}

define <8 x float> @add4(<8 x float> %a) nounwind readonly ssp {
entry:
  %tmp1 = load <8 x float>* @gb, align 32
  %add = fadd <8 x float> %tmp1, %a
  ret <8 x float> %add
}

define <8 x float> @add5(<8 x float> %a) nounwind readonly ssp {
entry:
  %tmp1 = load <8 x float>** @pgb, align 8
  %tmp2 = load <8 x float>* %tmp1, align 32
  %add = fadd <8 x float> %tmp2, %a
  ret <8 x float> %add
}
