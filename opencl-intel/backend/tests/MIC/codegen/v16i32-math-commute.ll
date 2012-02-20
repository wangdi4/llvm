; XFAIL: win32
; XFAIL:
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@g = common global <16 x i32> zeroinitializer, align 64

define <16 x i32> @add1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: 	vaddpi	{{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpaddd	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %add = add nsw <16 x i32> %tmp1, %a
  ret <16 x i32> %add
}

define <16 x i32> @add2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vaddpi {{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpaddd	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %add = add nsw <16 x i32> %a, %tmp
  ret <16 x i32> %add
}

define <16 x i32> @mul1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: 	vmullpi	{{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpmulld	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %tmp1, %a
  ret <16 x i32> %mul
}

define <16 x i32> @mul2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: 	vmullpi	{{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpmulld	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %a, %tmp
  ret <16 x i32> %mul
}

define <16 x i32> @sub1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: 	vsubpi	{{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpsubd	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %a, %tmp1
  ret <16 x i32> %sub
}

define <16 x i32> @sub2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: 	vsubrpi	{{[^(]+}}(%rip), %v0, %v0
;
; KNC: 	vpsubrd	{{\(%r[a-z]+\)}}, %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %tmp, %a
  ret <16 x i32> %sub
}

define <16 x i32> @div1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vstored	%v0, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm0, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %div = sdiv <16 x i32> %a, %tmp1
  ret <16 x i32> %div
}

define <16 x i32> @div2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNF: vstored	%v0, {{[0-9]*}}(%rsp)
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: movl {{[0-9]*}}(%rsp), %eax
; KNF: cltd
; KNF: idivl
; KNF: movl %eax
; KNF: vloadd {{[0-9]*}}(%rsp), %v0
;
; KNC: vmovdqa32	%zmm0, {{[0-9]*}}(%rsp)
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{[0-9]*}}(%rsp), %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: vmovdqa32 {{[0-9]*}}(%rsp), %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %div = sdiv <16 x i32> %tmp, %a
  ret <16 x i32> %div
}
