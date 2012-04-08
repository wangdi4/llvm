; XFAIL: win32
; These currently fail because the VPANDNQ patterns fail to match because the all 1 const loads have multiple uses.  See FB case 1333.
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @andn1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vandnpq
; KNF: vandnpq
  %not = xor <16 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %and = and <16 x i64> %not, %b
  ret <16 x i64> %and
}

define <16 x i64> @andn2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vandnpq {{\(%[a-z]+\)}}
; KNF: vandnpq 64{{\(%[a-z]+\)}}
  %not = xor <16 x i64> %b, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <16 x i64>* %a, align 128
  %and = and <16 x i64> %tmp1, %not
  ret <16 x i64> %and
}

define <16 x i64> @andn3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vandnpq {{\(%[a-z]+\)}}
; KNF: vandnpq 64{{\(%[a-z]+\)}}
  %not = xor <16 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp2 = load <16 x i64>* %b, align 128
  %and = and <16 x i64> %tmp2, %not
  ret <16 x i64> %and
}

define <16 x i64> @andn4(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vandnpq ([[R1]])
; KNF: vandnpq 64([[R1]])
  %not = xor <16 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <16 x i64>* @gb, align 128
  %and = and <16 x i64> %tmp1, %not
  ret <16 x i64> %and
}

define <16 x i64> @andn5(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vandnpq ([[R2]])
; KNF: vandnpq 64([[R2]])
  %not = xor <16 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %and = and <16 x i64> %tmp2, %not
  ret <16 x i64> %and
}
