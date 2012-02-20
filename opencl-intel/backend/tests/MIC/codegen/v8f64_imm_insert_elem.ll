; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s
define <8 x double> @test_shuffle(<8 x double> %arr, double %val){
  %1 = insertelement <8 x double> %arr, double %val, i32 7
; CHECK: vkmov %eax, %k1
; CHECK: vshuf128x32 $68, $0, %v1, %v0{%k1}
  ret <8 x double> %1
}
