; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8
declare <16 x float> @llvm.x86.mic.round.ps(<16 x float>, i8, i8)

define <16 x float> @round1(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vroundps $1, $2, {{%v[0-9]+}}, {{%v[0-9]+}}
  %round = call <16 x float> @llvm.x86.mic.round.ps(<16 x float> %a, i8 2, i8 1)
  ret <16 x float> %round
}

define <16 x float> @round2(<16 x float>* %p) nounwind readnone ssp {
entry:
; KNF: vroundps $1, $2, ({{%r[a-z0-9]+}}), {{%v[0-9]+}}
  %a = load <16 x float>* %p
  %round = call <16 x float> @llvm.x86.mic.round.ps(<16 x float> %a, i8 2, i8 1)
  ret <16 x float> %round
}

