; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@g = common global <16 x i64> zeroinitializer, align 128

define <16 x i64> @add1(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: add1
; CHECK-NOT: addq
; CHECK: ret
  %tmp1 = load <16 x i64>* @g, align 128
  %add = add nsw <16 x i64> %tmp1, %a
  ret <16 x i64> %add
}

define <16 x i64> @add2(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: add2
; CHECK-NOT: addq
; CHECK: ret
  %tmp = load <16 x i64>* @g, align 128
  %add = add nsw <16 x i64> %a, %tmp
  ret <16 x i64> %add
}

define <16 x i64> @mul1(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: mul1
; CHECK-NOT: imulq
; CHECK: ret
  %tmp1 = load <16 x i64>* @g, align 128
  %mul = mul <16 x i64> %tmp1, %a
  ret <16 x i64> %mul
}

define <16 x i64> @mul2(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: mul2
; CHECK-NOT: imulq
; CHECK: ret
  %tmp = load <16 x i64>* @g, align 128
  %mul = mul <16 x i64> %a, %tmp
  ret <16 x i64> %mul
}

define <16 x i64> @sub1(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: sub1
; CHECK-NOT: subq
; CHECK: ret
  %tmp1 = load <16 x i64>* @g, align 128
  %sub = sub <16 x i64> %a, %tmp1
  ret <16 x i64> %sub
}

define <16 x i64> @sub2(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: sub2
; CHECK-NOT: subq
; CHECK: ret
  %tmp = load <16 x i64>* @g, align 128
  %sub = sub <16 x i64> %tmp, %a
  ret <16 x i64> %sub
}
