; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s
 
define i1 @testbt( i32 %shift, i32 %mask){
; CHECK: btl %edi, %esi
; CHECK: setnc %al
  %1 = shl i32 1, %shift
  %2 = and i32 %1, %mask
  %ret = icmp eq i32 %2, 0
  ret i1 %ret
}

define i1 @testnbt( i32 %shift, i32 %mask){
; CHECK: btl %edi, %esi
; CHECK: setc %al
  %1 = shl i32 1, %shift
  %2 = and i32 %1, %mask
  %ret = icmp ne i32 %2, 0
  ret i1 %ret
}
