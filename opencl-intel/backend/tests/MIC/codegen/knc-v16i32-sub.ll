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

define <16 x i32> @sub1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNC: vpsubd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %sub = sub nsw <16 x i32> %a, %b
  ret <16 x i32> %sub
}

define <16 x i32> @sub2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNC: vpsubrd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <16 x i32>* %a, align 64
  %sub = sub nsw <16 x i32> %tmp1, %b
  ret <16 x i32> %sub
}

define <16 x i32> @sub3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: vpsubd {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp2 = load <16 x i32>* %b, align 64
  %sub = sub nsw <16 x i32> %a, %tmp2
  ret <16 x i32> %sub
}

define <16 x i32> @sub4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: vpsubd {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <16 x i32>* @gb, align 64
  %sub = sub nsw <16 x i32> %a, %tmp1
  ret <16 x i32> %sub
}

define <16 x i32> @sub5(<16 x i32> %a) nounwind readonly ssp {
entry:

; KNC: movq   pgb(%rip), [[R1:%[a-z]+]] 
; KNC: vpsubd ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %sub = sub nsw <16 x i32> %a, %tmp2
  ret <16 x i32> %sub
}
