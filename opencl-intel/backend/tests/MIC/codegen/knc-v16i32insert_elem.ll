; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s
define <16 x i32> @test_shuffle(<16 x i32> %arr, i32 %val, i32 %index){
  %1 = insertelement <16 x i32> %arr, i32 %val, i32 %index
; CHECK: vmovdqa32 %zmm0, -64(%rsp)
; CHECK: movl      %edi, -64(%rsp,%rsi,4)
; CHECK: vmovapd   -64(%rsp), %zmm0
  ret <16 x i32> %1
}
