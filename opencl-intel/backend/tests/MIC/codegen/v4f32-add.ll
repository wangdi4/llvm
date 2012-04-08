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

@gb = common global <4 x float> zeroinitializer, align 16
@pgb = common global <4 x float>* null, align 8

define <4 x float> @add1(<4 x float> %a, <4 x float> %b) nounwind readnone ssp {
entry:
  %add = fadd <4 x float> %a, %b
  ret <4 x float> %add
}

define <4 x float> @add2(<4 x float>* nocapture %a, <4 x float> %b) nounwind readonly ssp {
entry:
  %tmp1 = load <4 x float>* %a, align 16
  %add = fadd <4 x float> %tmp1, %b
  ret <4 x float> %add
}

define <4 x float> @add3(<4 x float> %a, <4 x float>* nocapture %b) nounwind readonly ssp {
entry:
  %tmp2 = load <4 x float>* %b, align 16
  %add = fadd <4 x float> %tmp2, %a
  ret <4 x float> %add
}

define <4 x float> @add4(<4 x float> %a) nounwind readonly ssp {
entry:
  %tmp1 = load <4 x float>* @gb, align 16
  %add = fadd <4 x float> %tmp1, %a
  ret <4 x float> %add
}

define <4 x float> @add5(<4 x float> %a) nounwind readonly ssp {
entry:
  %tmp1 = load <4 x float>** @pgb, align 8
  %tmp2 = load <4 x float>* %tmp1, align 16
  %add = fadd <4 x float> %tmp2, %a
  ret <4 x float> %add
}
