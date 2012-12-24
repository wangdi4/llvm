; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s --check-prefix=KNC
define <8 x i64> @test_insert_el(<8 x i64> %arr, i64 %val){
  %1 = insertelement <8 x i64> %arr, i64 %val, i32 1

; KNC: movl $12,
; KNC: movq %rdi, -8(%rsp)
; KNC: kmov
; KNC: valignd
; KNC: ret

  ret <8 x i64> %1
}
