; RUN: llc < %s -mcpu=knc -march=y86-64 | FileCheck %s



define <16 x i32> @sdiv_zero(<16 x i32> %var) {
entry:
; CHECK: sdiv_zero
; CHECK-NOT sra
; CHECK: ret
  %0 = sdiv <16 x i32> %var, zeroinitializer
  ret <16 x i32> %0
}


define <16 x i32> @sdiv16x32(<16 x i32> %var) {
entry:
; CHECK: sdiv16x32
; CHECK: vpsrad  $31
; CHECK: vpsrld  $30
; CHECK: vpaddd
; CHECK: vpsrad  $2
; CHECK: ret
  %a0 = sdiv <16 x i32> %var, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  ret <16 x i32> %a0
}

define <16 x i32> @sdiv16_negative(<16 x i32> %var) {
entry:
; CHECK: sdiv16_negative
; CHECK: vpsrad  $31
; CHECK: vpsrld  $30
; CHECK: vpaddd
; CHECK: vpsrad  $2
; CHECK: vpsubd
; CHECK: ret
  %a0 = sdiv <16 x i32> %var, <i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4, i32 -4>
  ret <16 x i32> %a0
}
