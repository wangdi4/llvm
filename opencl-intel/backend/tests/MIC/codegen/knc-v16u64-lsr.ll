; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @shiftright1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright1:
; KNC: vpshufd $160, %zmm2, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, %zmm3, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm1{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %shr = lshr <16 x i64> %a, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright2:
; KNC: vpshufd $160, %zmm0, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} (%rdi), [[Z1:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 [[Z1]]{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, %zmm1, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} 64(%rdi), [[Z2:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 [[Z2]]{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %tmp1 = load <16 x i64>* %a
  %shr = lshr <16 x i64> %tmp1, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright3:
; KNC: vpshufd $160, (%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, 64(%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm1{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %tmp2 = load <16 x i64>* %b
  %shr = lshr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright4(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright4:
; KNC: vpshufd $160, gb(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, {{[a-z]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm1{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %tmp1 = load <16 x i64>* @gb, align 128
  %shr = lshr <16 x i64> %a, %tmp1
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright5(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright5:
; KNC: mov{{[a-z]+}} pgb(%rip), [[R1:%[a-z]+]]
; KNC: vpshufd $160, ([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, 64([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm1{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %tmp1 = load <16 x i64>** @pgb
  %tmp2 = load <16 x i64>* %tmp1
  %shr = lshr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright6(<16 x i64> %a) nounwind readnone ssp {
entry:
; Only 15 shrq's since the shift by 0 is optimized away
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq
; KNF: shrq

; KNC: shiftright6:
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm0{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm1{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd

  %shr = lshr <16 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  ret <16 x i64> %shr
}

define <16 x i64> @shiftleft7(<16 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftleft7:
; KNC: vpslld $4, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vpsrld $28, %zmm0, [[Z2:%zmm[0-9]+]]
; KNC: vpord  [[Z2]]{cdab}, [[Z1]], %zmm0
; KNC: vpslld $4, %zmm1, [[Z3:%zmm[0-9]+]]
; KNC: vpsrld $28, %zmm1, [[Z4:%zmm[0-9]+]]
; KNC: vpord  [[Z4]]{cdab}, [[Z3]], %zmm1

  %shr = lshr <16 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  ret <16 x i64> %shr
}

define <16 x i64> @shiftleft8(<16 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftleft8:
; KNC: vpslld $5, %zmm0{cdab}, %zmm0
; KNC: vpxord %zmm0, %zmm0, %zmm0{k{{[0-9]+}}}
; KNC: vpslld $5, %zmm1{cdab}, %zmm1
; KNC: vpxord %zmm1, %zmm1, %zmm1{k{{[0-9]+}}}

  %shr = lshr <16 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  ret <16 x i64> %shr
}
