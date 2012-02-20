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

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @shiftright1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vsrlpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsrlvd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shr = lshr <16 x i32> %a, %b
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNF: vloadd ({{%[a-z]+}}), [[R1:%v[0-9]+]]
; KNF: vsrlpi {{%v[0-9]+}}, [[R1]], {{%v[0-9]+}}
;
; KNC: vmovdqa32 ({{%[a-z]+}}), [[R1:%zmm[0-9]+]]
; KNC: vpsrlvd {{%zmm[0-9]+}}, [[R1]], {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %shr = lshr <16 x i32> %tmp1, %b
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsrlpi ({{%[a-z]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsrlvd ({{%[a-z]+}}), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %shr = lshr <16 x i32> %a, %tmp2
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vsrlpi 
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpsrlvd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* @gb, align 64
  %shr = lshr <16 x i32> %a, %tmp1
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: movq 
; KNF: vsrlpi 
;
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vpsrlvd ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %shr = lshr <16 x i32> %a, %tmp2
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright6(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vsrlpi {{[^(]+\(%rip\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsrlvd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shr = lshr <16 x i32> %a, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i32> %shr
}

define <16 x i32> @shiftright7(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vsrlpi {{[^(]+\(%rip\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsrld $5, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shr = lshr <16 x i32> %a, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  ret <16 x i32> %shr
}
