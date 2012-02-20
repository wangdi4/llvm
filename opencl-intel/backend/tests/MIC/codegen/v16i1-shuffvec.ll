; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; ModuleID = 'Program'

target datalayout = "e-p:64:64"

define <16 x i1> @____Vectorized_.scan_separated_args(i32 %val) nounwind alwaysinline {
entry:
  %0 = icmp eq i32 %val, 1
; KNF: negl [[R1:%[a-z0-9]+]]
; KNF: vkmov [[R1]], %k1
  %1 = insertelement <16 x i1> undef, i1 %0, i32 0
  %2 = shufflevector <16 x i1> %1, <16 x i1> undef, <16 x i32> zeroinitializer
  ret <16 x i1> %2
}
