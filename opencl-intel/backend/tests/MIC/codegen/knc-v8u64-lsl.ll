
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s -check-prefix=KNC 

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

; 64-bit logical shift left by vector on KNF implemented through sequence of 2 32-bit logical shift left: vsllpi
define <8 x i64> @shiftleft1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft1:
; KNC: vpshufd $160, %zmm1, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %shl = shl <8 x i64> %a, %b
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft2:
; KNC: vpshufd $160, %zmm0, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} (%rdi), [[Z1:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 [[Z1]]{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %tmp1 = load <8 x i64>* %a
  %shl = shl <8 x i64> %tmp1, %b
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft3:
; KNC: vpshufd $160, (%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %tmp2 = load <8 x i64>* %b
  %shl = shl <8 x i64> %a, %tmp2
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft4:
; KNC: vpshufd $160, gb(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %tmp1 = load <8 x i64>* @gb, align 64
  %shl = shl <8 x i64> %a, %tmp1
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft5:
; KNC: mov{{[a-z]+}} pgb(%rip), [[R1:%[a-z]+]]
; KNC: vpshufd $160, ([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %tmp1 = load <8 x i64>** @pgb
  %tmp2 = load <8 x i64>* %tmp1
  %shl = shl <8 x i64> %a, %tmp2
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft6(<8 x i64> %a) nounwind readnone ssp {
entry:
; KNF: vsllpi
; KNF: vsllpi

; KNC: shiftleft6:
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsrlvd
; KNC: vpsllvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsllvd

  %shl = shl <8 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  ret <8 x i64> %shl
}


