; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8
declare <16 x float> @llvm.x86.mic.scale.ps(<16 x float>, <16 x i32>)

define <16 x float> @scale1(<16 x float> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vscaleps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %scale = call <16 x float> @llvm.x86.mic.scale.ps(<16 x float> %a, <16 x i32> %b)
  ret <16 x float> %scale
}

define <16 x float> @scale2(<16 x float> %a, <16 x i32>* %p) nounwind readnone ssp {
entry:
; KNF: vscaleps ({{%r[a-z0-9]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
  %b = load <16 x i32>* %p
  %scale = call <16 x float> @llvm.x86.mic.scale.ps(<16 x float> %a, <16 x i32> %b)
  ret <16 x float> %scale
}

