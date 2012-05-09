; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;

target datalayout = "e-p:64:64"

define <16 x i32> @cvt(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pi
;
; KNC: vcvtfxpntps2dq $0, %zmm0, %zmm0
  %conv = fptosi <16 x float> %a to <16 x i32>
  ret <16 x i32> %conv
}

@g = common global <16 x float> zeroinitializer, align 64

define <16 x i32> @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtps2pi	$0, {rz}, (%r
;
; KNC: vcvtfxpntps2dq $0, g(%rip), %zmm0 
  %i = load <16 x float>* @g
  %conv = fptosi <16 x float> %i to <16 x i32>
  ret <16 x i32> %conv
}
