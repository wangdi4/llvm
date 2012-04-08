; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

@g = common global <16 x i64> zeroinitializer, align 128
@pg = common global <16 x i64>* null, align 8

define <16 x i64> @negate1(<16 x i64> %a) nounwind readnone ssp {
entry:
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
  %sub = sub nsw <16 x i64> zeroinitializer, %a
  ret <16 x i64> %sub
}

define <16 x i64> @negate2() nounwind readonly ssp {
entry:
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
  %tmp = load <16 x i64>* @g, align 128
  %sub = sub nsw <16 x i64> zeroinitializer, %tmp
  ret <16 x i64> %sub
}

define <16 x i64> @negate3() nounwind readonly ssp {
entry:
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
  %tmp = load <16 x i64>** @pg, align 8
  %tmp1 = load <16 x i64>* %tmp, align 128
  %sub = sub nsw <16 x i64> zeroinitializer, %tmp1
  ret <16 x i64> %sub
}
