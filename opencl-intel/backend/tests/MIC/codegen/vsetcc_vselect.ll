; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

define <16 x i32> @func(<16 x i32> %v1, <16 x i32> %v2, <16 x i32> %v3, <16 x i32> %v4) nounwind {
; KNF:  vcmppi    {eq}, %v1, %v0, %k1
; KNF:  vorpi     %v2, %v2, %v3{%k1}
  %t0 = icmp eq <16 x i32> %v1, %v2
  %t1 = select <16 x i1> %t0, <16 x i32> %v3, <16 x i32> %v4
  ret <16 x i32> %t1
}

