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

; 64-bit logical shift left by vector on KNF implemented through sequence of 2 32-bit logical shift left: vsllpi
define <8 x i64> @shiftleft1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %shl = shl <8 x i64> %a, %b
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %tmp1 = load <8 x i64>* %a
  %shl = shl <8 x i64> %tmp1, %b
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %tmp2 = load <8 x i64>* %b
  %shl = shl <8 x i64> %a, %tmp2
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %tmp1 = load <8 x i64>* @gb, align 64
  %shl = shl <8 x i64> %a, %tmp1
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %tmp1 = load <8 x i64>** @pgb
  %tmp2 = load <8 x i64>* %tmp1
  %shl = shl <8 x i64> %a, %tmp2
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft6(<8 x i64> %a) nounwind readnone ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi
  %shl = shl <8 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  ret <8 x i64> %shl
}
