; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <8 x i64> @func (i32 %val) nounwind alwaysinline {
; KNF: vloadq    -8(%rsp){1to8}, %v0
  %0 = zext i32 %val to i64
  %temp = insertelement <8 x i64> undef, i64 %0, i32 0
  %vector = shufflevector <8 x i64> %temp, <8 x i64> undef, <8 x i32> zeroinitializer
  ret <8 x i64> %vector
}
