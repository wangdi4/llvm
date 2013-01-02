; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

@g = common global <8 x i64> zeroinitializer, align 128
@pg = common global <8 x i64>* null, align 8

define <8 x i64> @negate1(<8 x i64> %a) nounwind readnone ssp {
entry:
; CHECK: negate1
; CHECK-NOT: subq
; CHECK: ret
  %sub = sub nsw <8 x i64> zeroinitializer, %a
  ret <8 x i64> %sub
}

define <8 x i64> @negate2() nounwind readonly ssp {
entry:
; CHECK: negate2
; CHECK-NOT: subq
; CHECK: ret
  %tmp = load <8 x i64>* @g, align 128
  %sub = sub nsw <8 x i64> zeroinitializer, %tmp
  ret <8 x i64> %sub
}

define <8 x i64> @negate3() nounwind readonly ssp {
entry:
; CHECK: negate3
; CHECK-NOT: subq
; CHECK: ret
  %tmp = load <8 x i64>** @pg, align 8
  %tmp1 = load <8 x i64>* %tmp, align 128
  %sub = sub nsw <8 x i64> zeroinitializer, %tmp1
  ret <8 x i64> %sub
}
