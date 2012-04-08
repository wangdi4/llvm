; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"


define <16 x i1> @cmpoeq(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpoeq
; KNF: vcmppd	{eq},
; KNF: vcmppd	{eq},
; KNF: vkmovlhb
  %mask = fcmp oeq <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpogt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpogt
; KNF: vcmppd	{lt},
; KNF: vcmppd	{lt},
; KNF: vkmovlhb
  %mask = fcmp ogt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpoge(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpoge
; KNF: vcmppd	{le},
; KNF: vcmppd	{le},
; KNF: vkmovlhb
  %mask = fcmp oge <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpolt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpolt
; KNF: vcmppd	{lt},
; KNF: vcmppd	{lt},
; KNF: vkmovlhb

  %mask = fcmp olt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpole(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpole
; KNF: vcmppd	{le},
; KNF: vcmppd	{le},
; KNF: vkmovlhb
  %mask = fcmp ole <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpone(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpone
; KNF: vcmppd	{ord},
; KNF: vcmppd	{neq},
; KNF: vcmppd	{ord},
; KNF: vcmppd	{neq},
; KNF: vkmovlhb
  %mask = fcmp one <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpord(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpord
; KNF: vcmppd	{ord},
; KNF: vcmppd	{ord},
; KNF: vkmovlhb
  %mask = fcmp ord <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpueq(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpueq
; KNF: vcmppd	{unord},
; KNF: vcmppd	{eq},
; KNF: vcmppd	{unord},
; KNF: vcmppd	{eq},
; KNF: vkmovlhb
  %mask = fcmp ueq <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpugt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpugt
; KNF: vcmppd	{nle},
; KNF: vcmppd	{nle},
; KNF: vkmovlhb
  %mask = fcmp ugt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpuge(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpuge
; KNF: vcmppd	{nlt},
; KNF: vcmppd	{nlt},
; KNF: vkmovlhb
  %mask = fcmp uge <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpult(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpult
; KNF: vcmppd	{nle},
; KNF: vcmppd	{nle},
; KNF: vkmovlhb
  %mask = fcmp ult <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpule(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpule
; KNF: vcmppd	{nlt},
; KNF: vcmppd	{nlt},
; KNF: vkmovlhb
  %mask = fcmp ule <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpune(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpune
; KNF: vcmppd	{neq},
; KNF: vcmppd	{neq},
; KNF: vkmovlhb
  %mask = fcmp une <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpunord(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: @cmpunord
; KNF: vcmppd	{unord},
; KNF: vcmppd	{unord},
; KNF: vkmovlhb
  %mask = fcmp uno <16 x double> %a, %b
  ret <16 x i1> %mask
}

