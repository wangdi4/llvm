; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s
define <16 x i32> @test_shuffle(<16 x i32> %arr, i32 %val, i32 %index){
  %1 = insertelement <16 x i32> %arr, i32 %val, i32 %index
; CHECK: shll %cl, %eax
; CHECK-NEXT: vkmov %eax, %k1
; CHECK-NEXT: vshuf128x32 $0, $0, %v1, %v0{%k1}
  ret <16 x i32> %1
}
