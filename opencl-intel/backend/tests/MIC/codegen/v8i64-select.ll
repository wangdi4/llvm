; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8
declare <8 x i64> @llvm.x86.mic.blend.pq(<8 x i1>, <8 x i64>, <8 x i64>)

define <8 x i64> @sel1(i1 %m, <8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
  %sel = select i1 %m, <8 x i64> %a, <8 x i64> %b
  ret <8 x i64> %sel
}

define <8 x i64> @sel2(<8 x i1> %m, <8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
  %sel = select <8 x i1> %m, <8 x i64> %a, <8 x i64> %b
  ret <8 x i64> %sel
}

;define <8 x i64> @sel3(<8 x i1> %m, <8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
;entry:
; KNFc: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
;  %sel = call <8 x i64> @llvm.x86.mic.blend.pq(<8 x i1> %m, <8 x i64> %a, <8 x i64> %b)
;  ret <8 x i64> %sel
;}
