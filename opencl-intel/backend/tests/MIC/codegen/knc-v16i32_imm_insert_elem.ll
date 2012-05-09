; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s --check-prefix=KNC
define <16 x i32> @test_shuffle(<16 x i32> %arr, i32 %val){
  %1 = insertelement <16 x i32> %arr, i32 %val, i32 5
  ret <16 x i32> %1
; CHECK: vkmov %eax, %k1
; CHECK: vshuf128x32 $0, $0, %v1, %v0{%k1}
;
; KNC: movl $32,
; KNC: kmov
; KNC: vbroadcastss -8(%rsp),
; KNC-NOT: valignd
; KNC: vmovaps   %zmm1, %zmm0{%k1}
; KNC: ret
}
