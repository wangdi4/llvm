; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s -check-prefix=KNC 

target datalayout = "e-p:64:64"

define <8 x i64> @shiftright7(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftright7:
; KNC: vpsrld $4, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vpslld $28, %zmm0, [[Z2:%zmm[0-9]+]]
; KNC: vpord  [[Z2]]{cdab}, [[Z1]], %zmm0

  %shr = lshr <8 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  ret <8 x i64> %shr
}

define <8 x i64> @shiftright8(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftright8:
; KNC: vpsrld $5, %zmm0{cdab}, %zmm0
; KNC: vpxord %zmm0, %zmm0, %zmm0{k{{[0-9]+}}}

  %shr = lshr <8 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  ret <8 x i64> %shr
}
