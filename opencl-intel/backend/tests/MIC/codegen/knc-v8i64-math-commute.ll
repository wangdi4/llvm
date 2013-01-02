; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@g = common global <8 x i64> zeroinitializer, align 64

define <8 x i64> @add1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: add1
; CHECK-NOT: addq
; CHECK: ret
  %tmp1 = load <8 x i64>* @g, align 64
  %add = add nsw <8 x i64> %tmp1, %a
  ret <8 x i64> %add
}

define <8 x i64> @mul1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: mul1 
; CHECK-NOT: imulq
; CHECK: ret
  %tmp1 = load <8 x i64>* @g, align 64
  %mul = mul <8 x i64> %tmp1, %a
  ret <8 x i64> %mul
}

define <8 x i64> @sub1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: sub1
; CHECK-NOT: subq
; CHECK: ret
  %tmp1 = load <8 x i64>* @g, align 64
  %sub = sub <8 x i64> %a, %tmp1
  ret <8 x i64> %sub
}
