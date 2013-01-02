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

define <16 x i32> @add1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNC: add1:
; KNC: vpaddd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %add = add nsw <16 x i32> %a, %b
  ret <16 x i32> %add
}

define <16 x i32> @add2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNC: add2:
; KNC: vpaddd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %add = add nsw <16 x i32> %tmp1, %b
  ret <16 x i32> %add
}

define <16 x i32> @add3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: add3:
; KNC: vpaddd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %add = add nsw <16 x i32> %tmp2, %a
  ret <16 x i32> %add
}

define <16 x i32> @add4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: add4:
; KNC: vpaddd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* @gb, align 64
  %add = add nsw <16 x i32> %tmp1, %a
  ret <16 x i32> %add
}

define <16 x i32> @add5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: add5:
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpaddd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %add = add nsw <16 x i32> %tmp2, %a
  ret <16 x i32> %add
}
