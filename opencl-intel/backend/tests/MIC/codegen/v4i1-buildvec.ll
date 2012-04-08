; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <4 x i1> @allZero_v4(<4 x i1> %t)  nounwind readnone {
entry:
;  KNF:  vknot %k1, %k1
  %res = xor <4 x i1> %t, <i1 true, i1 true, i1 true, i1 true>
  ret <4 x i1> %res
}
