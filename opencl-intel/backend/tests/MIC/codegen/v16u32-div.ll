; XFAIL: win32
; XFAIL: *
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

define <16 x i32> @rem1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vstored	{{%v[0-9]}}, {{[0-9]*}}(%rsp)
; KNF: vstored	{{%v[0-9]}}, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{[0-9]*}}(%rsp)
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %div = udiv <16 x i32> %a, %b
  ret <16 x i32> %div
}

define <16 x i32> @rem2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNF: vstored	%v1, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm1, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp1 = load <16 x i32>* %a, align 64
  %div = udiv <16 x i32> %tmp1, %b
  ret <16 x i32> %div
}

define <16 x i32> @rem3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vstored	%v0, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm0, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp2 = load <16 x i32>* %b, align 64
  %div = udiv <16 x i32> %a, %tmp2
  ret <16 x i32> %div
}

define <16 x i32> @rem4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vstored	%v0, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm0, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp1 = load <16 x i32>* @gb, align 64
  %div = udiv <16 x i32> %a, %tmp1
  ret <16 x i32> %div
}

define <16 x i32> @rem5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vstored	%v0, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: xorl %edx, %edx
; KNF: divl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm0, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: xorl %edx, %edx
; KNC: divl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %div = udiv <16 x i32> %a, %tmp2
  ret <16 x i32> %div
}
