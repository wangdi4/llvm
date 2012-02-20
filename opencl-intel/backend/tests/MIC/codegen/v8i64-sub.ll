; XFAIL: win32
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

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @sub1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: vsubsetbpi
; CHECK: vsbbpi
  %sub = sub nsw <8 x i64> %a, %b
  ret <8 x i64> %sub
}

define <8 x i64> @sub2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: vsubsetbpi
; CHECK: vsbbpi
  %tmp1 = load <8 x i64>* %a, align 64
  %sub = sub nsw <8 x i64> %tmp1, %b
  ret <8 x i64> %sub
}

define <8 x i64> @sub3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: vsubsetbpi
; CHECK: vsbbpi
  %tmp2 = load <8 x i64>* %b, align 64
  %sub = sub nsw <8 x i64> %a, %tmp2
  ret <8 x i64> %sub
}

define <8 x i64> @sub4(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: vsubsetbpi
; CHECK: vsbbpi
  %tmp1 = load <8 x i64>* @gb, align 64
  %sub = sub nsw <8 x i64> %a, %tmp1
  ret <8 x i64> %sub
}

define <8 x i64> @sub5(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: vsubsetbpi
; CHECK: vsbbpi
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %sub = sub nsw <8 x i64> %a, %tmp2
  ret <8 x i64> %sub
}
