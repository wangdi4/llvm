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

define <16 x i32> @A(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vxorpq %v0, %v0, %v0
;
; KNC: vpxorq %zmm0, %zmm0, %zmm0
  ret <16 x i32> zeroinitializer
}

define <8 x i64> @B(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vxorpq %v0, %v0, %v0
;
; KNC: vpxorq %zmm0, %zmm0, %zmm0
  ret <8 x i64> zeroinitializer
}

define <16 x float> @C(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vxorpq %v0, %v0, %v0
;
; KNC: vpxorq %zmm0, %zmm0, %zmm0
  ret <16 x float> zeroinitializer
}

define <8 x double> @D(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNF: vxorpq %v0, %v0, %v0
;
; KNC: vpxorq %zmm0, %zmm0, %zmm0
  ret <8 x double> zeroinitializer
}
