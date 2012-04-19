; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s
define <8 x i64> @test_shuffle(<8 x i64> %arr, i64 %val){
  %1 = insertelement <8 x i64> %arr, i64 %val, i32 1
; CHECK: vkmov %eax, %k1
; CHECK: vshuf128x32 $68, $0, %v1, %v0{%k1}
  ret <8 x i64> %1
}
