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

; Note: these is no MOVD instruction on MIC, but these tests check the
; equivalent sequence
define <16 x i32> @A(i32 %X) nounwind uwtable readnone {
; KNF: vxorpi    [[R0:%v[0-9]+]]
; KNF: vloadd    -8(%rsp){1to16}, [[R1:%v[0-9]+]]
; KNF: vshuf128x32 $0, $0, [[R1]], [[R0]]{[[R2:%k[1-9]+]]}
  %a0 = insertelement <16 x i32> undef, i32 %X, i32 0
  %a1 = insertelement <16 x i32> %a0, i32 0, i32 1
  ret <16 x i32> %a1
}

define <16 x float> @B(float %X) nounwind uwtable readnone {
; KNF: vxorpi    [[R0:%v[0-9]+]]
; KNF-NOT: vloadd
; KNF: vshuf128x32 $0, $0, [[R1:%v[0-9]+]], [[R0]]{[[R2:%k[1-9]+]]}
  %a0 = insertelement <16 x float> undef, float %X, i32 0
  %a1 = insertelement <16 x float> %a0, float 0.0, i32 1
  ret <16 x float> %a1
}
