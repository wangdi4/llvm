; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @or1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vporq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %or = or <16 x i64> %a, %b
  ret <16 x i64> %or
}

define <16 x i64> @or2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vorpq ([[R1:%[a-z]+]]), {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vorpq 64([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vporq ([[R1:%[a-z]+]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq 64([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i64>* %a, align 128
  %or = or <16 x i64> %tmp1, %b
  ret <16 x i64> %or
}

define <16 x i64> @or3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vorpq ([[R1:%[a-z]+]]), {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vorpq 64([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vporq ([[R1:%[a-z]+]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq 64([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i64>* %b, align 128
  %or = or <16 x i64> %tmp2, %a
  ret <16 x i64> %or
}

define <16 x i64> @or4(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vorpq ([[R2:%[a-z]+]]), {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vorpq 64([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vporq ([[R2:%[a-z]+]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq 64([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i64>* @gb, align 128
  %or = or <16 x i64> %tmp1, %a
  ret <16 x i64> %or
}

define <16 x i64> @or5(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vorpq ([[R2]])
; KNF: vorpq 64([[R2]])
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vporq ([[R2]])
; KNC: vporq 64([[R2]])
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %or = or <16 x i64> %tmp2, %a
  ret <16 x i64> %or
}
