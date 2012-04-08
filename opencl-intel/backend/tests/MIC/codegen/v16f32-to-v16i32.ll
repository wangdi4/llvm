; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x i32> @cvt(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vcvtps2pi
  %conv = fptosi <16 x float> %a to <16 x i32>
  ret <16 x i32> %conv
}

@g = common global <16 x float> zeroinitializer, align 64

define <16 x i32> @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtps2pi	$0, {rz}, (%r
  %i = load <16 x float>* @g
  %conv = fptosi <16 x float> %i to <16 x i32>
  ret <16 x i32> %conv
}

declare <16 x i32> @llvm.x86.mic.cvt.ps2pi(<16 x float>, i8, i8)

define <16 x i32> @icvt(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: icvt:
; KNF: vcvtps2pi        $3, $2, {{%v[0-9]+}}
  %conv = call <16 x i32> @llvm.x86.mic.cvt.ps2pi(<16 x float> %a, i8 2, i8 3)
  ret <16 x i32> %conv
}

define <16 x i32> @icvtm() nounwind readnone ssp {
entry:
; KNF: icvtm:
; KNF: vcvtps2pi	$3, $2, (%r
  %i = load <16 x float>* @g
  %conv = call <16 x i32> @llvm.x86.mic.cvt.ps2pi(<16 x float> %i, i8 2, i8 3)
  ret <16 x i32> %conv
}
