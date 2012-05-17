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
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright1:
; KNC: vpshufd $160, %zmm2, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, %zmm3, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm1, [[Z2:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm1{cdab}, [[Z2]]
; KNC: vpsravd
; KNC: vpsravd

  %shr = ashr <16 x i64> %a, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright2:
; KNC: vpshufd $160, %zmm0, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} (%rdi), [[Z1:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, [[Z1]], [[Z2:%zmm[0-9]+]]
; KNC: vmovdqa32 [[Z1]]{cdab}, [[Z2]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, %zmm1, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} 64(%rdi), [[Z3:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, [[Z3]], [[Z4:%zmm[0-9]+]]
; KNC: vmovdqa32 [[Z3]]{cdab}, [[Z4]]
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>* %a, align 128
  %shr = ashr <16 x i64> %tmp1, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright3:
; KNC: vpshufd $160, (%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, 64(%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm1, [[Z2:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm1{cdab}, [[Z2]]
; KNC: vpsravd
; KNC: vpsravd

  %tmp2 = load <16 x i64>* %b, align 128
  %shr = ashr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright4(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright4:
; KNC: vpshufd $160, gb(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, {{[a-z]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm1, [[Z2:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm1{cdab}, [[Z2]]
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>* @gb, align 128
  %shr = ashr <16 x i64> %a, %tmp1
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright5(<16 x i64> %a) nounwind readonly ssp {
entry:
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright5:
; KNC: mov{{[a-z]+}} pgb(%rip), [[R1:%[a-z]+]]
; KNC: vpshufd $160, ([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, 64([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %shr = ashr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright6(<16 x i64> %a) nounwind readnone ssp {
entry:
; Only 15 sarq's since the shift by 0 is optimized away
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq
; KNF: sarq

; KNC: shiftright6:
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm1, [[Z2:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm1{cdab}, [[Z2]]
; KNC: vpsravd
; KNC: vpsravd

  %shr = ashr <16 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright7(<16 x i64> %a) nounwind readnone ssp {

entry:

; KNC: shiftright7:
; KNC: vpsrld $4
; KNC: vpslld $28
; KNC: vpsrad $4 
; KNC: vpsrld $4
; KNC: vpslld $28
; KNC: vpsrad $4 

  %shr = ashr <16 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright8(<16 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftright8:
; KNC: vpsrad $5
; KNC: vpsrad $31
; KNC: vpsrad $5
; KNC: vpsrad $31

  %shr = ashr <16 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  ret <16 x i64> %shr
}

