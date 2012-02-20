; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8
declare <16 x float> @llvm.x86.mic.getexp.ps(<16 x float>)

define <16 x float> @getexp1(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vgetexpps {{%v[0-9]+}}, {{%v[0-9]+}}
  %getexp = call <16 x float> @llvm.x86.mic.getexp.ps(<16 x float> %a)
  ret <16 x float> %getexp
}

define <16 x float> @getexp2(<16 x float>* %p) nounwind readnone ssp {
entry:
; KNF: vgetexpps ({{%r[a-z0-9]+}}), {{%v[0-9]+}}
  %a = load <16 x float>* %p
  %getexp = call <16 x float> @llvm.x86.mic.getexp.ps(<16 x float> %a)
  ret <16 x float> %getexp
}

