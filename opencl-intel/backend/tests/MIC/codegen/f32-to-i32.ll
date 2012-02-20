; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x i32> @cvt_vec(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pi
  %conv = fptosi <16 x float> %a to <16 x i32>
  ret <16 x i32> %conv
}


define i32 @cvt(float %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pi
  %conv = fptosi float %a to i32
  ret i32 %conv
}

@g = common global float 0.0, align 4

define i32 @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtps2pi
  %i = load float* @g
  %conv = fptosi float %i to i32
  ret i32 %conv
}
