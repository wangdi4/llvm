; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s

define i1 @testbt( i64 %shift, i64 %mask){
; CHECK: btq %rdi, %rsi
; CHECK: setnc %al
  %1 = shl i64 1, %shift
  %2 = and i64 %1, %mask
  %ret = icmp eq i64 %2, 0
  ret i1 %ret
}

define i1 @testnbt( i64 %shift, i64 %mask){
; CHECK: btq %rdi, %rsi
; CHECK: setc %al
  %1 = shl i64 1, %shift
  %2 = and i64 %1, %mask
  %ret = icmp ne i64 %2, 0
  ret i1 %ret
}

