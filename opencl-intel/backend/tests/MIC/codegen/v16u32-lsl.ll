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

define <16 x i32> @shiftleft1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vsllpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsllvd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shl = shl <16 x i32> %a, %b
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNF: vloadd ({{%[a-z]+}}), [[R1:%v[0-9]+]]
; KNF: vsllpi {{%v[0-9]+}}, [[R1]], {{%v[0-9]+}}
;
; KNC: vmovdqa32 ({{%[a-z]+}}), [[R1:%zmm[0-9]+]]
; KNC: vpsllvd {{%zmm[0-9]+}}, [[R1]], {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %shl = shl <16 x i32> %tmp1, %b
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsllpi ({{%[a-z]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsllvd ({{%[a-z]+}}), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %shl = shl <16 x i32> %a, %tmp2
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vsllpi 
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpsllvd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* @gb, align 64
  %shl = shl <16 x i32> %a, %tmp1
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: movq
; KNF: vsllpi 
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: movq ([[R1]]), [[R2:%[a-z]+]]
; KNC: vpsllvd ([[R2]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %shl = shl <16 x i32> %a, %tmp2
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft6(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vsllpi {{[^(]+\(%rip\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpsllvd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shl = shl <16 x i32> %a, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i32> %shl
}

define <16 x i32> @shiftleft7(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNF: vsllpi {{[^(]+\(%rip\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpslld $2, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %shl = shl <16 x i32> %a, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  ret <16 x i32> %shl
}
