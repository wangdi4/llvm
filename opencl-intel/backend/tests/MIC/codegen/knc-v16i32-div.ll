; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

perf bug

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @div1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNC: div1:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %div = sdiv <16 x i32> %a, %b
  ret <16 x i32> %div
}

define <16 x i32> @div2(<16 x i32>* nocapture %a, <16 x i32> %b) nounwind readonly ssp {
entry:
; KNC: div2:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* %a, align 64
  %div = sdiv <16 x i32> %tmp1, %b
  ret <16 x i32> %div
}

define <16 x i32> @div3(<16 x i32> %a, <16 x i32>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: div3:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp2 = load <16 x i32>* %b, align 64
  %div = sdiv <16 x i32> %a, %tmp2
  ret <16 x i32> %div
}



define <16 x i32> @div4(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: div4:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>* @gb, align 64
  %div = sdiv <16 x i32> %a, %tmp1
  ret <16 x i32> %div
}



define <16 x i32> @div5(<16 x i32> %a) nounwind readonly ssp {
entry:
; KNC: div5:
; KNC: vmovdqa32	{{%zmm[0-9]}}, {{-?[0-9]*}}({{%[a-z]+}})
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: cltd
; KNC: idivl
; KNC: vmovapd {{-?[0-9]*}}({{%[a-z]+}}), %zmm0
; KNC: ret
  %tmp1 = load <16 x i32>** @pgb, align 8
  %tmp2 = load <16 x i32>* %tmp1, align 64
  %div = sdiv <16 x i32> %a, %tmp2
  ret <16 x i32> %div
}


