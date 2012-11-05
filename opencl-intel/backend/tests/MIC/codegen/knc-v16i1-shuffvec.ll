; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc    \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;
; ModuleID = 'Program'

target datalayout = "e-p:64:64"

define <16 x i1> @____Vectorized_.scan_separated_args(i32 %val) nounwind alwaysinline {
entry:
; KNC: negl [[R1:%[a-z0-9]+]]
; KNC: kmov [[R1]], %k1
; KNC: ret
  %0 = icmp eq i32 %val, 1
  %1 = insertelement <16 x i1> undef, i1 %0, i32 0
  %2 = shufflevector <16 x i1> %1, <16 x i1> undef, <16 x i32> zeroinitializer
  ret <16 x i1> %2
}

define <16 x i1> @B(<16 x i1> %a) {
; KNC:  kmov      %k1, [[R1:%[a-z0-9]+]]
; KNC:  andl      $1, [[R1]]
; KNC:  negl      [[R1]]
; KNC:  kmov      [[R1]]
; KNC:  ret
  %res = shufflevector <16 x i1> %a, <16 x i1> undef, <16 x i32> zeroinitializer
  ret <16 x i1> %res
}
