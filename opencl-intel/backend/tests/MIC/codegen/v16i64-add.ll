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

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @add1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %add = add nsw <16 x i64> %a, %b
  ret <16 x i64> %add
}

define <16 x i64> @add2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp1 = load <16 x i64>* %a, align 128
  %add = add nsw <16 x i64> %tmp1, %b
  ret <16 x i64> %add
}

define <16 x i64> @add3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp2 = load <16 x i64>* %b, align 128
  %add = add nsw <16 x i64> %tmp2, %a
  ret <16 x i64> %add
}

define <16 x i64> @add4(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp1 = load <16 x i64>* @gb, align 128
  %add = add nsw <16 x i64> %tmp1, %a
  ret <16 x i64> %add
}

define <16 x i64> @add5(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %add = add nsw <16 x i64> %tmp2, %a
  ret <16 x i64> %add
}
