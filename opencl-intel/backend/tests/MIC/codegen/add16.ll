; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s

define void @f(i16* %p){
  %val = load i16* %p
  %sum = add i16 %val, 1
  store i16 %sum, i16* %p
; CHECK: incw
; CHECK-NEXT: ret
  ret void
}
