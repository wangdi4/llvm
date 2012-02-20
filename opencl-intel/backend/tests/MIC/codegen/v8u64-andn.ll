; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @and1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vandnpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpandnq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %not = xor <8 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %and = and <8 x i64> %not, %b
  ret <8 x i64> %and
}

define <8 x i64> @and2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vandnpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpandnq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %not = xor <8 x i64> %b, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <8 x i64>* %a, align 64
  %and = and <8 x i64> %tmp1, %not
  ret <8 x i64> %and
}

define <8 x i64> @and3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vandnpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpandnq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %not = xor <8 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp2 = load <8 x i64>* %b, align 64
  %and = and <8 x i64> %tmp2, %not
  ret <8 x i64> %and
}

define <8 x i64> @and4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vandnpq ([[R1]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpandnq ([[R1]]), {{%zmm[0-9]+}}
  %not = xor <8 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <8 x i64>* @gb, align 64
  %and = and <8 x i64> %tmp1, %not
  ret <8 x i64> %and
}

define <8 x i64> @and5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vandnpq ([[R2]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vpandnq ([[R2]]), {{%zmm[0-9]+}}
  %not = xor <8 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %and = and <8 x i64> %not, %tmp2
  ret <8 x i64> %and
}
