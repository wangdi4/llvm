; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s --check-prefix=KNC
define <16 x i32> @test_insert_el(<16 x i32> %arr, i32 %val){
  %1 = insertelement <16 x i32> %arr, i32 %val, i32 5
  ret <16 x i32> %1
; KNC: movl $32,
; KNC: kmov
; KNC: valignd
; KNC: ret
}
