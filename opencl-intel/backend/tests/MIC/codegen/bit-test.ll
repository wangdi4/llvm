; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc | FileCheck %s

define i32 @FillBuffers(i32 %a, i32 %b) {
; CHECK: btl
; CHECK: ret
entry:                                     
  %shl = shl i32 1, %a
  %and = and i32 %shl, %b
  %if = icmp eq i32 %and, 0
  br i1 %if, label %__true, label %__false

__true:                                        
  ret i32 1

__false: 
  ret i32 0                                       
}

define i32 @FillBuffers2(i32 %a, i32 %b) {
; CHECK: btl
entry:                                     
  %shl = shl i32 1, %a
  %and = and i32 %shl, %b
  %if = icmp eq i32 %and, 0
  %res = select i1 %if, i32 100, i32 50
  ret i32 %res                                     
}

