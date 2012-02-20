; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8
declare <16 x i32> @llvm.x86.mic.blend.pi(<16 x i1>, <16 x i32>, <16 x i32>)

define <16 x i32> @sel1(i1 %m, <16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
  %sel = select i1 %m, <16 x i32> %a, <16 x i32> %b
  ret <16 x i32> %sel
}

define <16 x i32> @sel2(<16 x i1> %m, <16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
  %sel = select <16 x i1> %m, <16 x i32> %a, <16 x i32> %b
  ret <16 x i32> %sel
}

;define <16 x i32> @sel3(<16 x i1> %m, <16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
;entry:
; KNFc: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
;  %sel = call <16 x i32> @llvm.x86.mic.blend.pi(<16 x i1> %m, <16 x i32> %a, <16 x i32> %b)
;  ret <16 x i32> %sel
;}
