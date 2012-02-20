; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x float> @cvt_vec(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2ps  $0, %v0, %v0
  %conv = sitofp <16 x i32> %a to <16 x float>
  ret <16 x float> %conv
}

define float @cvt(i32 %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2ps
  %conv = sitofp i32 %a to float
  ret float %conv
}

@g = common global i32 0, align 4

define float @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtpi2ps
  %i = load i32* @g
  %conv = sitofp i32 %i to float
  ret float %conv
}
