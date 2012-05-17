; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s -check-prefix=KNC 

define <8 x i64> @shiftright2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:

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

  %tmp1 = load <8 x i64>* %a
  %shr = ashr <8 x i64> %tmp1, %b
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright7(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftright7:
; KNC: vpsrld $4
; KNC: vpslld $28
; KNC: vpsrad $4 

  %shr = ashr <8 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright8(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftright8:
; KNC: vpsrad $5
; KNC: vpsrad $31

  %shr = ashr <8 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  ret <8 x i64> %shr
}
