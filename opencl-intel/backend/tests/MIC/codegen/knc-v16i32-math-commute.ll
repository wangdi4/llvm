; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@g = common global <16 x i32> zeroinitializer, align 64

define <16 x i32> @add1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpaddd	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %add = add nsw <16 x i32> %tmp1, %a
  ret <16 x i32> %add
}

define <16 x i32> @add2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpaddd	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %add = add nsw <16 x i32> %a, %tmp
  ret <16 x i32> %add
}

define <16 x i32> @mul1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpmulld	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %tmp1, %a
  ret <16 x i32> %mul
}

define <16 x i32> @mul2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpmulld	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %a, %tmp
  ret <16 x i32> %mul
}

define <16 x i32> @sub1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpsubd	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %a, %tmp1
  ret <16 x i32> %sub
}

define <16 x i32> @sub2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpsubrd	{{[^(]+}}(%rip), %zmm0, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %tmp, %a
  ret <16 x i32> %sub
}

define <16 x i32> @div1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: vmovdqa32	%zmm0, {{-?[0-9]*\(%r[sb]p\)}}
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: vmovapd {{-?[0-9]*\(%r[sb]p\)}}, %zmm0
  %tmp1 = load <16 x i32>* @g, align 64
  %div = sdiv <16 x i32> %a, %tmp1
  ret <16 x i32> %div
}

define <16 x i32> @div2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: vmovdqa32	%zmm0, {{-?[0-9]*\(%r[sb]p\)}}
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: movl {{-?[a-z0-9%()]+}}, %eax
; KNC: cltd
; KNC: idivl
; KNC: movl %eax
; KNC: vmovapd {{-?[0-9]*\(%r[sb]p\)}}, %zmm0
  %tmp = load <16 x i32>* @g, align 64
  %div = sdiv <16 x i32> %tmp, %a
  ret <16 x i32> %div
}
