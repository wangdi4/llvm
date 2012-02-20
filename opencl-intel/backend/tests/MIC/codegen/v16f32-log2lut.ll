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
declare <16 x float> @llvm.x86.mic.log2lut.ps(<16 x i32>)

define <16 x float> @log2lut1(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vlog2lutps {{%v[0-9]+}}, {{%v[0-9]+}}
  %log2lut = call <16 x float> @llvm.x86.mic.log2lut.ps(<16 x i32> %a)
  ret <16 x float> %log2lut
}

define <16 x float> @log2lut2(<16 x i32>* %p) nounwind readnone ssp {
entry:
; KNF: vlog2lutps ({{%r[a-z0-9]+}}), {{%v[0-9]+}}
  %a = load <16 x i32>* %p
  %log2lut = call <16 x float> @llvm.x86.mic.log2lut.ps(<16 x i32> %a)
  ret <16 x float> %log2lut
}

