; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x i1> @cmpeq(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpeq
; KNF: vcmppi	{eq},
; KNF: vcmppi	{eq},
; KNF: vcmppi	{eq},
; KNF: vcmppi	{eq},
; KNF: vkmovlhb
  %mask = icmp eq <16 x i64> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpgt(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpgt
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nle},
; KNF: vkor
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nle},
; KNF: vkor
; KNF: vkmovlhb
  %mask = icmp sgt <16 x i64> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpge(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpge
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nlt},
; KNF: vkor
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nlt},
; KNF: vkor
; KNF: vkmovlhb
  %mask = icmp sge <16 x i64> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmplt(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmplt
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{lt},
; KNF: vkor
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{lt},
; KNF: vkor
; KNF: vkmovlhb
  %mask = icmp slt <16 x i64> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmple(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmple
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{le},
; KNF: vkor
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{le},
; KNF: vkor
; KNF: vkmovlhb
  %mask = icmp sle <16 x i64> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpne(<16 x i64> %a, <16 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpne
; KNF: vcmppi	{neq},
; KNF: vcmppi	{neq},
; KNF: vkor
; KNF: vcmppi	{neq},
; KNF: vcmppi	{neq},
; KNF: vkor
; KNF: vkmovlhb
  %mask = icmp ne <16 x i64> %a, %b
  ret <16 x i1> %mask
}

