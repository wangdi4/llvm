; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s
;
define <8 x double> @test_shuffle(<8 x double> %arr, double %val, i32 %index){
  %1 = insertelement <8 x double> %arr, double %val, i32 %index
; CHECK: vmovdqa64 %zmm0, -64(%rsp)
; CHECK: vpackstorelpd %zmm1, -64(%rsp,%rdi,8){%k1}
; CHECK: vmovapd   -64(%rsp), %zmm0
  ret <8 x double> %1
}
