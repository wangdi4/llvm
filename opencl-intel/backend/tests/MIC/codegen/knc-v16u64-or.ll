; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @or1(<16 x i64> %a, <16 x i64> %b, <16 x i64>* nocapture %c) nounwind readnone ssp {
entry:
; KNC: vporq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %or = or <16 x i64> %a, %b
  store <16 x i64> %or, <16 x i64>* %c
  ret <16 x i64> %or
}

define <16 x i64> @or2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; KNC: vporq ([[R1:%[a-z]+]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq 64([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i64>* %a, align 128
  %or = or <16 x i64> %tmp1, %b
  store <16 x i64> %or, <16 x i64>* %a
  ret <16 x i64> %or
}

define <16 x i64> @or3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: vporq ([[R1:%[a-z]+]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq 64([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i64>* %b, align 128
  %or = or <16 x i64> %tmp2, %a
  store <16 x i64> %or, <16 x i64>* %b
  ret <16 x i64> %or
}

define <16 x i64> @or4(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: vporq {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: vporq {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i64>* @gb, align 128
  %or = or <16 x i64> %tmp1, %a
  store <16 x i64> %or, <16 x i64>* %b
  ret <16 x i64> %or
}

define <16 x i64> @or5(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vporq ([[R1]])
; KNC: vporq 64([[R1]])
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %or = or <16 x i64> %tmp2, %a
  store <16 x i64> %or, <16 x i64>* %b
  ret <16 x i64> %or
}
