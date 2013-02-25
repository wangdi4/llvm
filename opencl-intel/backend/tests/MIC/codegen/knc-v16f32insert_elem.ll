; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s

define <16 x float> @test_shuffle(<16 x float> %arr, float %val, i32 %index){
  %1 = insertelement <16 x float> %arr, float %val, i32 %index
; CHECK: vmovdqa32 %zmm0, -64(%rsp)
; CHECK: vpackstorelps %zmm1, -64(%rsp,%rdi,4){%k1}
; CHECK: vmovapd   -64(%rsp), %zmm0
  ret <16 x float> %1
}
