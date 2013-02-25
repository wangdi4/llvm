; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;

target datalayout = "e-p:64:64"

define <16 x float> @cvt(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpi2ps
  %conv = sitofp <16 x i32> %a to <16 x float>
  ret <16 x float> %conv
}

@g = common global <16 x i32> zeroinitializer, align 64

define <16 x float> @cvtm() nounwind readnone ssp {
entry:
; KNF: vcvtpi2ps	$0, (%r
  %i = load <16 x i32>* @g
  %conv = sitofp <16 x i32> %i to <16 x float>
  ret <16 x float> %conv
}

declare <16 x float> @llvm.x86.mic.cvt.pi2ps(<16 x i32>, i8)

define <16 x float> @icvt(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: icvt:
; KNF: vcvtpi2ps        $2, {{%v[0-9]+}}
  %conv = call <16 x float> @llvm.x86.mic.cvt.pi2ps(<16 x i32> %a, i8 2)
  ret <16 x float> %conv
}

define <16 x float> @icvtm() nounwind readnone ssp {
entry:
; KNF: icvtm:
; KNF: vcvtpi2ps	$2, (%r
  %i = load <16 x i32>* @g
  %conv = call <16 x float> @llvm.x86.mic.cvt.pi2ps(<16 x i32> %i, i8 2)
  ret <16 x float> %conv
}
