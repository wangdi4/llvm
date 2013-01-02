; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @and1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNC: and1:
; KNC: vpandd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %and = and <16 x i32> %a, %b
  ret <16 x i32> %and
}

define <16 x i32> @and2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNC: and2:
; KNC: vpandd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %and = and <16 x i32> %tmp1, %b
  ret <16 x i32> %and
}

define <16 x i32> @and3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: and3:
; KNC: vpandd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %and = and <16 x i32> %tmp2, %a
  ret <16 x i32> %and
}

define <16 x i32> @and4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: and4:
; KNC: vpandd gb(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @gb, align 64
  %and = and <16 x i32> %tmp1, %a
  ret <16 x i32> %and
}

define <16 x i32> @and5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: and5:
; KNC: movq pgb(%rip), [[R2:%[a-z]+]]
; KNC: vpandd ([[R2]]), %zmm0, %zmm0
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %and = and <16 x i32> %tmp2, %a
  ret <16 x i32> %and
}

