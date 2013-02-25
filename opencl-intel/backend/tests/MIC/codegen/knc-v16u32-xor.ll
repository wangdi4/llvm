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

define <16 x i32> @xor1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNC: xor1:
;
; KNC: vpxord {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %xor = xor <16 x i32> %a, %b
  ret <16 x i32> %xor
}

define <16 x i32> @xor2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNC: xor2:
;
; KNC: vpxord {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>* %a, align 64
  %xor = xor <16 x i32> %tmp1, %b
  ret <16 x i32> %xor
}

define <16 x i32> @xor3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: xor3:
;
; KNC: vpxord {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
  %tmp2 = load <16 x i32>* %b, align 64
  %xor = xor <16 x i32> %tmp2, %a
  ret <16 x i32> %xor
}

define <16 x i32> @xor4(<16 x i32> %a) nounwind readonly ssp {
entry:
;
; KNC: xor4:
; KNC: vpxord {{[^(]+\(%rip\)}}, %zmm0, %zmm0
; KNC: xor4
  %tmp1 = load <16 x i32>* @gb, align 64
  %xor = xor <16 x i32> %tmp1, %a
  ret <16 x i32> %xor
}

define <16 x i32> @xor5(<16 x i32> %a) nounwind readonly ssp {
entry:
;
; KNC: xor5:
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpxord ([[R1]]), %zmm0, {{%zmm[0-9]+}}
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %xor = xor <16 x i32> %tmp2, %a
  ret <16 x i32> %xor
}
