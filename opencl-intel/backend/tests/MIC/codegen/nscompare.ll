; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s

define void @f(i8* %p, i8 %c){
  %b = icmp sgt i8 %c, -1
  %res = select i1 %b, i8 0, i8 3
  store i8 %res, i8* %p, align 1
; CHECK: testb
; CHECK: js
  ret void
}
