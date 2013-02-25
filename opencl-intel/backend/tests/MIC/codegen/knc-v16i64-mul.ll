; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @mul1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: mul1
; CHECK-NOT: imulq
; CHECK: ret
  %mul = mul nsw <16 x i64> %a, %b
  ret <16 x i64> %mul
}

define <16 x i64> @mul2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: mul2
; CHECK-NOT: imulq
; CHECK: ret
  %tmp1 = load <16 x i64>* %a, align 128
  %mul = mul nsw <16 x i64> %tmp1, %b
  ret <16 x i64> %mul
}

define <16 x i64> @mul3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: mul3
; CHECK-NOT: imulq
; CHECK: ret
  %tmp2 = load <16 x i64>* %b, align 128
  %mul = mul nsw <16 x i64> %tmp2, %a
  ret <16 x i64> %mul
}

define <16 x i64> @mul4(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: mul4
; CHECK-NOT: imulq
; CHECK: ret
  %tmp1 = load <16 x i64>* @gb, align 128
  %mul = mul nsw <16 x i64> %tmp1, %a
  ret <16 x i64> %mul
}

define <16 x i64> @mul5(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: mul5
; CHECK-NOT: imulq
; CHECK: ret
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %mul = mul nsw <16 x i64> %tmp2, %a
  ret <16 x i64> %mul
}
