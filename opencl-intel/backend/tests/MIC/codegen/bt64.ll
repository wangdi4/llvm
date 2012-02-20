; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %s

define i1 @testbt( i64 %shift, i64 %mask){
; CHECK: movl %edi, %ecx
; CHECK: shlq %cl, %rdx
; CHECK: testq %rdx, %rsi 
  %1 = shl i64 1, %shift
  %2 = and i64 %1, %mask
  %ret = icmp eq i64 %2, 0
  ret i1 %ret
}

