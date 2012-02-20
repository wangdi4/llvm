; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <8 x i32> @cvt_vec(<8 x float> %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pu
  %conv = fptoui <8 x float> %a to <8 x i32>
  ret <8 x i32> %conv
}

define i32 @cvt(float %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pu
  %conv = fptoui float %a to i32
  ret i32 %conv
}

@g = common global float 0.0, align 4

define i32 @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtps2pu
  %i = load float* @g
  %conv = fptoui float %i to i32
  ret i32 %conv
}
