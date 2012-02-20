; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s
define <16 x float> @test_shuffle(<16 x float> %arr, float %val){
  %1 = insertelement <16 x float> %arr, float %val, i32 3
; CHECK: vkmov %eax, %k1
; CHECK-NEXT: vshuf128x32 $0, $0, %v1, %v0{%k1}
  ret <16 x float> %1
}
