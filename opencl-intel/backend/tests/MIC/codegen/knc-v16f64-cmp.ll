; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

;
;

target datalayout = "e-p:64:64"


define <16 x i1> @cmpoeq(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpoeq
; KNF: vcmppd	{eq},
; KNF: vcmppd	{eq},
; KNF: vkmovlhb
;
; KNC: vcmpeqpd
; KNC: vcmpeqpd
; KNC: kmerge2l1l
  %mask = fcmp oeq <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpogt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpogt
; KNF: vcmppd	{lt},
; KNF: vcmppd	{lt},
; KNF: vkmovlhb
;
; KNC: vcmpltpd 
; KNC: vcmpltpd
; KNC: kmerge2l1l
  %mask = fcmp ogt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpoge(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpoge
; KNF: vcmppd	{le},
; KNF: vcmppd	{le},
; KNF: vkmovlhb
;
; KNC: vcmplepd
; KNC: vcmplepd
; KNC: kmerge2l1l
  %mask = fcmp oge <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpolt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpolt
; KNF: vcmppd	{lt},
; KNF: vcmppd	{lt},
; KNF: vkmovlhb
;
; KNC: vcmpltpd 
; KNC: vcmpltpd
; KNC: kmerge2l1l

  %mask = fcmp olt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpole(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpole
; KNF: vcmppd	{le},
; KNF: vcmppd	{le},
; KNF: vkmovlhb
;
; KNC: vcmplepd 
; KNC: vcmplepd
; KNC: kmerge2l1l
  %mask = fcmp ole <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpone(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpone
; KNF: vcmppd	{neq},
; KNF: vcmppd	{ord},
; KNF: vcmppd	{neq},
; KNF: vcmppd	{ord},
; KNF: vkand
; KNF: vkand
; KNF: vkmovlhb
;
; KNC: vcmpneqpd 
; KNC: vcmpordpd
; KNC: vcmpneqpd 
; KNC: vcmpordpd
; KNC: kand
; KNC: kand
; KNC: kmerge2l1l
  %mask = fcmp one <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpord(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpord
; KNF: vcmppd	{ord},
; KNF: vcmppd	{ord},
; KNF: vkmovlhb
;
; KNC: vcmpordpd
; KNC: vcmpordpd
; KNC: kmerge2l1l
  %mask = fcmp ord <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpueq(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpueq
; KNF: vcmppd	{eq},
; KNF: vcmppd	{unord},
; KNF: vcmppd	{eq},
; KNF: vcmppd	{unord},
; KNF: vkor
; KNF: vkor
; KNF: vkmovlhb
;
; KNC: vcmpeqpd
; KNC: vcmpunordpd
; KNC: vcmpeqpd
; KNC: vcmpunordpd
; KNC: kor
; KNC: kor
; KNC: kmerge2l1l
  %mask = fcmp ueq <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpugt(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpugt
; KNF: vcmppd	{nle},
; KNF: vcmppd	{nle},
; KNF: vkmovlhb
;
; KNC: vcmpnlepd 
; KNC: vcmpnlepd
; KNC: kmerge2l1l
  %mask = fcmp ugt <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpuge(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpuge
; KNF: vcmppd	{nlt},
; KNF: vcmppd	{nlt},
; KNF: vkmovlhb
;
; KNC: vcmpnltpd
; KNC: vcmpnltpd
; KNC: kmerge2l1l
  %mask = fcmp uge <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpult(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpult
; KNF: vcmppd	{nle},
; KNF: vcmppd	{nle},
; KNF: vkmovlhb
;
; KNC: vcmpnlepd 
; KNC: vcmpnlepd
; KNC: kmerge2l1l
  %mask = fcmp ult <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpule(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpule
; KNF: vcmppd	{nlt},
; KNF: vcmppd	{nlt},
; KNF: vkmovlhb
;
; KNC: vcmpnltpd 
; KNC: vcmpnltpd
; KNC: kmerge2l1l
  %mask = fcmp ule <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpune(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpune
; KNF: vcmppd	{neq},
; KNF: vcmppd	{neq},
; KNF: vkmovlhb
;
; KNC: vcmpneqpd
; KNC: vcmpneqpd
; KNC: kmerge2l1l
  %mask = fcmp une <16 x double> %a, %b
  ret <16 x i1> %mask
}

define <16 x i1> @cmpunord(<16 x double> %a, <16 x double> %b) nounwind ssp {
entry:
; KNF: cmpunord
; KNF: vcmppd	{unord},
; KNF: vcmppd	{unord},
; KNF: vkmovlhb
;
; KNC: vcmpunordpd
; KNC: vcmpunordpd
; KNC: kmerge2l1l
  %mask = fcmp uno <16 x double> %a, %b
  ret <16 x i1> %mask
}

