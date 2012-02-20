; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

; KNF: vkxor %k1, %k1
define <16 x i1> @A() {
 ret <16 x i1> zeroinitializer
}

; KNF: vkxor %k1, %k1
define <8 x i1> @B() {
 ret <8 x i1> zeroinitializer
}

; KNF: vkxnor %k1, %k1
define <16 x i1> @C() {
 ret <16 x i1> <i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1>
}

; KNF: vkxnor %k1, %k1
define <8 x i1> @D() {
 ret <8 x i1> <i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1,i1 1>
}
