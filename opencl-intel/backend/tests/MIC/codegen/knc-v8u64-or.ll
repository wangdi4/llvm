; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @or1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vporq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %or = or <8 x i64> %a, %b
  ret <8 x i64> %or
}

define <8 x i64> @or2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vorpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vporq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>* %a, align 64
  %or = or <8 x i64> %tmp1, %b
  ret <8 x i64> %or
}

define <8 x i64> @or3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vorpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vporq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp2 = load <8 x i64>* %b, align 64
  %or = or <8 x i64> %tmp2, %a
  ret <8 x i64> %or
}

define <8 x i64> @or4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vorpq ([[R1]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vporq ([[R1]]), {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>* @gb, align 64
  %or = or <8 x i64> %tmp1, %a
  ret <8 x i64> %or
}

define <8 x i64> @or5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vorpq ([[R1]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vporq ([[R1]]), {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %or = or <8 x i64> %tmp2, %a
  ret <8 x i64> %or
}
