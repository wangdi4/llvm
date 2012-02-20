; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s

define i32 @test() {
  %ptr = alloca i32
  store i32 3, i32* %ptr
  br label %b
; CHECK: movl $3, -8(%rsp)
b:
; CHECK: movl -8(%rsp), %eax
  %val = load i32* %ptr
  ret i32 %val
}
