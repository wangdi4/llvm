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

define <16 x i32> @test1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vloadq _const_0(%rip), %v0
;
; KNC: vloadq _const_0(%rip), %zmm0
  ret <16 x i32> <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
}

define <16 x float> @test2(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vloadq _const_1(%rip), %v0
;
; KNC: vloadq _const_1(%rip), %zmm0
  ret <16 x float> <float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0>

}

define <1 x float> @test4(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vloadd _const_2(%rip){1to16}, %v0
;
; KNC: vloadd _const_2(%rip){1to16}, %zmm0
  ret <1 x float> <float 42.0>
}
