; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

define <16 x i1> @test0(<16 x i1>  %x, <16 x i1> %v1, <16 x i1> %v2) nounwind readnone {
entry:
; KNF: test0:
; KNF: vkand  %k1, %k2
; KNF: vkandn %k3, %k1
; KNF: vkor   %k2, %k1
        %.0 = select <16 x i1>  %x, <16 x i1> %v1, <16 x i1> %v2        ; <i32> [#uses=1]
        ret <16 x i1> %.0
}


define <16 x i1> @test1(i1 %x, <16 x i1> %v1, <16 x i1> %v2) nounwind readnone {
entry:
; KNF: test1:
; KNF: testb $1, %dil
; KNF: vkmov %k2, %k1
        %.0 = select i1 %x, <16 x i1> %v1, <16 x i1> %v2        ; <i32> [#uses=1]
        ret <16 x i1> %.0
}
