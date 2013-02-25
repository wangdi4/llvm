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
; KNC: 	vpaddd	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* @g, align 64
  %add = add <16 x i32> %tmp1, %a
  ret <16 x i32> %add
}

define <16 x i32> @add2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpaddd	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp = load <16 x i32>* @g, align 64
  %add = add <16 x i32> %tmp, %a
  ret <16 x i32> %add
}

define <16 x i32> @mul1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpmulld	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %tmp1, %a
  ret <16 x i32> %mul
}

define <16 x i32> @mul2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpmulld	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp = load <16 x i32>* @g, align 64
  %mul = mul <16 x i32> %tmp, %a
  ret <16 x i32> %mul
}

define <16 x i32> @sub1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpsubd	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %a, %tmp1
  ret <16 x i32> %sub
}

define <16 x i32> @sub2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: 	vpsubrd	g(%rip), %zmm0, %zmm0
; KNC: ret
  %tmp = load <16 x i32>* @g, align 64
  %sub = sub <16 x i32> %tmp, %a
  ret <16 x i32> %sub
}

define <16 x i32> @div1(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: div1:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* @g, align 64
  %div = udiv <16 x i32> %a, %tmp1
  ret <16 x i32> %div
}

define <16 x i32> @div2(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: div2:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: divl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp = load <16 x i32>* @g, align 64
  %div = udiv <16 x i32> %tmp, %a
  ret <16 x i32> %div
}
