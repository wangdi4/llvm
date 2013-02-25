; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s --check-prefix=KNC

define <8 x double> @A(<8 x double> %arr, double %val){
  %1 = insertelement <8 x double> %arr, double %val, i32 3
; CHECK: vkmov %eax, %k1
; CHECK: vshuf128x32 $68, $0, %v1, %v0{%k1}
;
; KNC: movl $192,
; KNC: kmov
; KNC: valignd $10, %zmm1, %zmm1, %zmm0{%k
  ret <8 x double> %1
}

define <8 x double> @B(<8 x double> %arr, double %val){
  %1 = insertelement <8 x double> %arr, double %val, i32 0
; CHECK: vkmov %eax, %k1
; CHECK: vshuf128x32 $68, $0, %v1, %v0{%k1}
;
; KNC:  movl      $1,
; KNC:  kmov
; KNC:  vmovapd   %zmm1, %zmm0{%k
  ret <8 x double> %1
}
