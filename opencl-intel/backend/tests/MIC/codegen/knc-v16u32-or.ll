; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @or1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; CHECK: vpord  %zmm1, %zmm0, %zmm0
  %or = or <16 x i32> %a, %b
  ret <16 x i32> %or
}

define <16 x i32> @or2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; CHECK: vpord     (%rdi), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* %a, align 64
  %or = or <16 x i32> %tmp1, %b
  ret <16 x i32> %or
}

define <16 x i32> @or3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: vpord     (%rdi), %zmm0, %zmm0
  %tmp2 = load <16 x i32>* %b, align 64
  %or = or <16 x i32> %tmp2, %a
  ret <16 x i32> %or
}

define <16 x i32> @or4(<16 x i32> %a) nounwind readonly ssp {
entry:
; CHECK:vpord     gb(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @gb, align 64
  %or = or <16 x i32> %tmp1, %a
  ret <16 x i32> %or
}

define <16 x i32> @or5(<16 x i32> %a) nounwind readonly ssp {
entry:
; CHECK: vpord     (%r{{[a-z]+}}), %zmm0, %zmm0
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %or = or <16 x i32> %tmp2, %a
  ret <16 x i32> %or
}
