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

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @and1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vandpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpandd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %and = and <16 x i32> %a, %b
  ret <16 x i32> %and
}

define <16 x i32> @and2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNF: vandpi {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpandd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %and = and <16 x i32> %tmp1, %b
  ret <16 x i32> %and
}

define <16 x i32> @and3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vandpi {{\(%[a-z]+\)}}, {{%v[0-9]+}}
;
; KNC: vpandd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %and = and <16 x i32> %tmp2, %a
  ret <16 x i32> %and
}

define <16 x i32> @and4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: vandpi ([[R1]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpandd ([[R1]]), {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* @gb, align 64
  %and = and <16 x i32> %tmp1, %a
  ret <16 x i32> %and
}

define <16 x i32> @and5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: vandpi ([[R2]]), {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vpandd ([[R2]]), {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %and = and <16 x i32> %tmp2, %a
  ret <16 x i32> %and
}

define <16 x i32> @and6(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vandnpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpandnd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %not = xor <16 x i32> %a, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %and = and <16 x i32> %not, %b
  ret <16 x i32> %and
}
