; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s
define <8 x i64> @test_shuffle(<8 x i64> %arr, i64 %val, i32 %index){
; CHECK: vmovdqa64 %zmm0, -64(%rsp)
; CHECK: movq      %rdi, -64(%rsp,%rsi,8)
; CHECK: vmovapd   -64(%rsp), %zmm0
  %1 = insertelement <8 x i64> %arr, i64 %val, i32 %index
  ret <8 x i64> %1
}
