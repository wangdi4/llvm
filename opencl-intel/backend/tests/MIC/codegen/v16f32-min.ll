; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8
declare <16 x float> @llvm.x86.mic.min.ps(<16 x float>, <16 x float>)

define <16 x float> @min1(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vminps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %min = call <16 x float> @llvm.x86.mic.min.ps(<16 x float> %a, <16 x float> %b)
  ret <16 x float> %min
}

define <16 x float> @min2(<16 x float> %a, <16 x float>* %p) nounwind readnone ssp {
entry:
; KNF: vminps ({{%r[a-z0-9]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
  %b = load <16 x float>* %p
  %min = call <16 x float> @llvm.x86.mic.min.ps(<16 x float> %a, <16 x float> %b)
  ret <16 x float> %min
}

