; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s -check-prefix=KNC 

target datalayout = "e-p:64:64"

define <8 x i64> @shiftleft7(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftleft7:
; KNC: vpslld $4, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vpsrld $28, %zmm0, [[Z2:%zmm[0-9]+]]
; KNC: vpord  [[Z2]]{cdab}, [[Z1]], %zmm0

  %shl = shl <8 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  ret <8 x i64> %shl
}

define <8 x i64> @shiftleft8(<8 x i64> %a) nounwind readnone ssp {
entry:

; KNC: shiftleft8:
; KNC: vpslld $5, %zmm0{cdab}, %zmm0
; KNC: vpxord %zmm0, %zmm0, %zmm0{k{{[0-9]+}}}

  %shl = shl <8 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  ret <8 x i64> %shl
}
