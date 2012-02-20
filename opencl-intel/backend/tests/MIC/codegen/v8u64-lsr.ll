; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

; 64-bit logical shift right by vector on KNF implemented through sequence of 2 32-bit logical shift right: vsrlpi
define <8 x i64> @shiftright1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %shr = lshr <8 x i64> %a, %b
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %tmp1 = load <8 x i64>* %a
  %shr = lshr <8 x i64> %tmp1, %b
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %tmp2 = load <8 x i64>* %b
  %shr = lshr <8 x i64> %a, %tmp2
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %tmp1 = load <8 x i64>* @gb, align 64
  %shr = lshr <8 x i64> %a, %tmp1
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %tmp1 = load <8 x i64>** @pgb
  %tmp2 = load <8 x i64>* %tmp1
  %shr = lshr <8 x i64> %a, %tmp2
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright6(<8 x i64> %a) nounwind readnone ssp {
entry:
; KNF: vsrlpi
; KNF: vsrlpi
  %shr = lshr <8 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  ret <8 x i64> %shr
}
