; XFAIL: win32
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

define <8 x i64> @xor1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vxorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %xor = xor <8 x i64> %a, %b
  ret <8 x i64> %xor
}

define <8 x i64> @xor2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vxorpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>* %a, align 64
  %xor = xor <8 x i64> %tmp1, %b
  ret <8 x i64> %xor
}

define <8 x i64> @xor3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vxorpq {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp2 = load <8 x i64>* %b, align 64
  %xor = xor <8 x i64> %tmp2, %a
  ret <8 x i64> %xor
}

define <8 x i64> @xor4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vxorpq {{[^(]+\(%rip\)}}, {{%v[0-9]+}}
;
; KNC: vxorpq {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>* @gb, align 64
  %xor = xor <8 x i64> %tmp1, %a
  ret <8 x i64> %xor
}

define <8 x i64> @xor5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vxorpq ([[R1]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vxorpq ([[R1]]), {{%zmm[0-9]+}}
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %xor = xor <8 x i64> %tmp2, %a
  ret <8 x i64> %xor
}
